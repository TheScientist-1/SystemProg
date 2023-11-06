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
        printf("%d -->%lc--> %d\n", transition.startState, transition.symbol,
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
    transitionalStates.states = (int *)malloc(config.initialSize * sizeof(int));
    if (transitionalStates.states == NULL) {
    }
    transitionalStates.size = 0;
    bool newStatesAdded = false;

    for (int i = 0; i < initialStates.size; i++) {
        int currentState = initialStates.states[i];
        for (int j = 0; j < config.automatonSize; j++) {
            if (config.transitions[j].startState == currentState) {
                int nextState = config.transitions[j].endState;
                if (!StateinList(nextState, reachedStates)) {
                    addStateToList(nextState, reachedStates);
                    if (!StateinList(nextState, &transitionalStates)) {
                        addStateToList(nextState, &transitionalStates);
                        newStatesAdded = true;
                    }
                }
            }
        }
    }

    if (newStatesAdded) {
        struct States *nextTransitionalStates = malloc(sizeof(struct States));
        nextTransitionalStates->states = malloc(config.initialSize * sizeof(int));
        nextTransitionalStates->size = 0;
        for (int i = 0; i < transitionalStates.size; i++) {
            addStateToList(transitionalStates.states[i], nextTransitionalStates);
        }

        struct States nextStates = findReachableStates(*nextTransitionalStates, reachedStates, config);

        free(nextTransitionalStates->states);
        free(nextTransitionalStates);
    }

    free(transitionalStates.states);
    return *reachedStates;
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

    for (int i = 0; i < stateList.size; i++) {
        printf("%d ", stateList.states[i]);
    }
    printf("\n");
}

int main() {
    setlocale(LC_ALL, "uk_UA.UTF-8");
    printf("Automata \n");

    struct AutomatonConfig config;
    parseConfiguration("test.txt", &config);

    struct States initialStates;
    initialStates.states = (int *)malloc(config.initialSize * sizeof(int));
    initialStates.size = 0;
    addStateToList(config.initialState, &initialStates);

    // Here, we should have w0, the predetermined word, read from the user or predefined
    wchar_t w0_string[MAX_WORD_SIZE];
    // For now, let's simulate that w0 is provided as user input
    printf("Enter word w0: ");
    fgetws(w0_string, MAX_WORD_SIZE, stdin);
    w0_string[wcslen(w0_string) - 1] = L'\0'; // Remove newline at the end

    struct Word w0;
    w0.characters = w0_string;
    w0.size = wcslen(w0_string);

    // Process the word w0
    int stateAfterW0 = findFinalState(w0, config, config.initialState);
    if (stateAfterW0 == -1){
        printf("Word w0 is not accepted\n");
        return 0;

    }
    else {printf("State reached after processing w0: %d\n", stateAfterW0);}


    struct States states_after_w0;
    states_after_w0.states = (int *)malloc(config.initialSize * sizeof(int));
    states_after_w0.size = 0;
    addStateToList(stateAfterW0, &states_after_w0);

    struct States states_after_w1;
    states_after_w1.states = (int *) malloc(config.initialSize * sizeof(int));
    states_after_w1.size = 0;

    struct States reachableStates = findReachableStates(states_after_w0, &states_after_w0, config);
    struct States reachableStates1 = findReachableStates(states_after_w0, &states_after_w1, config);

/*    printf("States reachable after processing w0: ");
    for (int i = 0; i < states_after_w0.size; ++i) {
        printf("%d ", states_after_w0.states[i]);
    }
    printf("\n");*/


    printf("States reachable after processing w1: ");
    printStateList(reachableStates1);

    int isProper = 0;
    int finalState = -1;
    for (int i = 0; i < reachableStates.size; ++i) {
        if (checkIfProperFinalState(reachableStates.states[i], config)) {
            isProper = 1;
            finalState = reachableStates.states[i];
            break;
        }
    }
    if (isProper) {
        printf("Final state %d reached\n", finalState);
        printf("Word w0w1 accepted\n");
    } else {
        printf("Final state not reached. Word w0w1 not accepted\n");
    }


    free(initialStates.states);
    free(states_after_w0.states);
    free(states_after_w1.states);
    free(config.finalStates);
    free(config.transitions);

    return 0;
}
