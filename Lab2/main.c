#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include "hashmap.h"

const int MAX_WORD_SIZE = 256;

struct StateTransition {
    int startState;
    int endState;
    wchar_t symbol;
};

struct Word {
    wchar_t *characters;
    int size;
};

struct States {
    int *states;
    int size;
};

struct AutomatonConfig {
    int initialSize;
    int initialState;
    int finalStatesSize;
    int automatonSize;
    int *finalStates;
    struct StateTransition *transitions;
};

uint64_t stateTransitionHash(const void *transition, uint64_t seed0, uint64_t seed1) {
    const struct StateTransition *currentTransition = transition;
    return hashmap_sip(&currentTransition->symbol, 1, seed0, seed1);
}

int stateTransitionCompare(const void *a, const void *b, void *userData) {
    const struct StateTransition *ua = a;
    const struct StateTransition *ub = b;
    if (ua->symbol == ub->symbol && ua->startState == ub->startState) {
        return 0;
    } else {
        return -1;
    }
}

struct Word concatenateWords(struct Word word1, struct Word word2, struct Word word3) {
    int totalSize = word1.size + word2.size + word3.size;

    wchar_t *concatenatedChars = (wchar_t *) malloc((totalSize + 1) * sizeof(wchar_t));

    if (concatenatedChars == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    wcscpy(concatenatedChars, word1.characters);
    wcscat(concatenatedChars, word2.characters);
    struct Word result;
    result.characters = concatenatedChars;
    result.size = totalSize;

    return result;
}

void parseConfiguration(const char *filename, struct AutomatonConfig *config) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    fscanf(file, "%d", &config->initialSize);
    fscanf(file, "%d", &config->initialState);

    config->finalStates = (int *) malloc(sizeof(int) * MAX_WORD_SIZE);
    config->transitions = (struct StateTransition *) malloc(sizeof(struct StateTransition) * MAX_WORD_SIZE);

    int state;
    int i = 0;
    while (fscanf(file, "%d", &state) == 1) {
        config->finalStates[i] = state;
        i++;
    }
    config->finalStatesSize = i;

    char line[MAX_WORD_SIZE];
    fgets(line, sizeof(line), file);

    i = 0;
    struct StateTransition transition;
    while (fscanf(file, "%d %lc %d", &transition.startState, &transition.symbol, &transition.endState) == 3) {
        printf("init.state: %d, symbol: %lc, fin.state: %d\n", transition.startState, transition.symbol,
               transition.endState);
        config->transitions[i] = transition;
        i++;
    }
    config->automatonSize = i;

    fclose(file);
}

bool StateinList(int state, const struct States *stateList) {
    for (int i = 0; i < stateList->size; i++) {
        if (state == stateList->states[i]) {
            return true;
        }
    }
    return false;
}

void addStateToList(int state, struct States *stateList) {
    if (!StateinList(state, stateList)) {
        stateList->states[stateList->size++] = state;
    }
}

struct States findReachableStates(struct States initialStates, struct States *reachedStates, struct AutomatonConfig config) {
    struct States transitionalStates;
    transitionalStates.states = (int *) malloc(config.initialSize * sizeof(int));
    transitionalStates.size = 0;
    bool isDone = true;
    for (int i = 0; i < initialStates.size; i++) {
        int currentState = initialStates.states[i];
        for (int j = 0; j < config.automatonSize; j++) {
            if (!StateinList(currentState, reachedStates)) {
                addStateToList(currentState, reachedStates);
            }
            if (config.transitions[j].startState == currentState &&
                config.transitions[j].startState != config.transitions[j].endState &&
                !StateinList(config.transitions[j].endState, reachedStates)) {
                addStateToList(config.transitions[j].endState, &transitionalStates);
                isDone = false;
            }
        }
    }
    if (!isDone) {
        struct States nextStates = findReachableStates(transitionalStates, reachedStates, config);
        for (int k = 0; k < nextStates.size; k++) {
            addStateToList(nextStates.states[k], reachedStates);
        }
    }

    for (int i = 0; i < initialStates.size; i++) {
        addStateToList(initialStates.states[i], reachedStates);
    }
    return transitionalStates;
}

int findFinalState(struct Word inputWord, struct AutomatonConfig config, int initialState) {
    int currentState = initialState;
    struct hashmap *map = hashmap_new(sizeof(struct StateTransition), 0, 0, 0, stateTransitionHash, stateTransitionCompare, NULL, NULL);

    for (int i = 0; i < config.automatonSize; i++) {
        hashmap_set(map, &config.transitions[i]);
    }

    for (int i = 0; i < inputWord.size; i++) {
        struct StateTransition nextTransition;
        nextTransition.symbol = inputWord.characters[i];
        nextTransition.startState = currentState;
        struct StateTransition *currentTransition = hashmap_get(map, &nextTransition);
        if (currentTransition == NULL) {
            return -1;
        }
        currentState = currentTransition->endState;
    }
    return currentState;
}

int checkIfProperFinalState(int finalState, struct AutomatonConfig config) {
    for (int i = 0; i < config.finalStatesSize; i++) {
        if (config.finalStates[i] == finalState) {
            return 1;
        }
    }
    return 0;
}

void printStateList(struct States stateList) {
    if (stateList.states == NULL || stateList.size <= 0) {
        printf("No states to print\n");
        return;
    }

    printf("Reachable states: ");
    for (int i = 0; i < stateList.size; i++) {
        printf("%d ", stateList.states[i]);
    }
    printf("\n");
}

int main() {
    setlocale(LC_ALL, "uk_UA.UTF-8");

    printf("Enter the input word: ");
    wchar_t inputBuffer[MAX_WORD_SIZE];
    fgetws(inputBuffer, sizeof(inputBuffer), stdin);
    inputBuffer[wcslen(inputBuffer) - 1] = L'\0';

    struct Word w0;
    w0.characters = inputBuffer;
    w0.size = wcslen(inputBuffer);

    struct AutomatonConfig config;
    parseConfiguration("test.txt", &config);

    struct States initialStates;
    initialStates.states = (int *) malloc(config.initialSize * sizeof(int));
    initialStates.size = 0;
    addStateToList(config.initialState, &initialStates);

    struct States reachedStates;
    reachedStates.states = (int *) malloc(config.initialSize * sizeof(int));
    reachedStates.size = 0;

    struct States statesAfter_w0;
    statesAfter_w0.states = (int *) malloc(config.initialSize * sizeof(int));
    statesAfter_w0.size = 0;

    struct States statesAfter_w1;
    statesAfter_w1.states = (int *) malloc(config.initialSize * sizeof(int));
    statesAfter_w1.size = 0;

    findReachableStates(initialStates, &reachedStates, config);
    printStateList(reachedStates);

    for (int i = 0; i < reachedStates.size; i++) {
        int stateAfter_w0 = findFinalState(w0, config, reachedStates.states[i]);
        if (stateAfter_w0 == -1) {
            continue;
        }
        addStateToList(stateAfter_w0, &statesAfter_w0);
    }
    printStateList(statesAfter_w0);
    findReachableStates(statesAfter_w0, &statesAfter_w1, config);
    printStateList(statesAfter_w1);


    int isProper = 0;
    for (int i = 0; i < statesAfter_w1.size; ++i) {
        if (checkIfProperFinalState(statesAfter_w1.states[i], config)) {
            isProper = 1;
            break;
        }
    }

    if (isProper) {
        printf("Accepted\n");
    } else {
        printf("Not accepted\n");
    }

    return 0;
}
