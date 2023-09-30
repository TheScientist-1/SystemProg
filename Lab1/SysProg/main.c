#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>

#define max_word_length 31

typedef struct {
    wchar_t **data;
    int size;
    int capacity;
} DynamicArray;

bool isUniqueChars(const wchar_t *str) {
    bool char_set[65536] = {false};
    for (int i = 0; i < wcslen(str); i++) {
        int val = (int) str[i];
        if(char_set[val]) return false;
        char_set[val] = true;
    }
    return true;
}

wchar_t *get_next_word(FILE *document) {
    wchar_t *new_word_placeholder = malloc(sizeof(wchar_t) * max_word_length);
    int pointer = 0;
    wint_t character = fgetwc(document);

    while (character != WEOF && (!iswalnum(character) && character != L'_')) {
        character = fgetwc(document);  // Skip non-alphanumeric characters
    }

    while (character != WEOF && (iswalnum(character) || character == L'_')) {
        new_word_placeholder[pointer] = character;
        pointer++;
        if (pointer == max_word_length - 1) {
            break;  // Avoid buffer overflow
        }
        character = fgetwc(document);
    }

    if (pointer == 0) {
        free(new_word_placeholder);
        return NULL;
    } else {
        new_word_placeholder[pointer] = L'\0';
        return new_word_placeholder;
    }
}

void initArray(DynamicArray *array, int capacity) {
    array->data = (wchar_t **)malloc(sizeof(wchar_t *) * capacity);
    array->size = 0;
    array->capacity = capacity;
}

void insertArray(DynamicArray *array, const wchar_t *element) {
    for (int i = 0; i < array->size; i++) {
        if (wcscmp(array->data[i], element) == 0) {
            return;  // Avoid duplicates
        }
    }
    if (array->size == array->capacity) {
        array->capacity *= 2;
        array->data = (wchar_t **)realloc(array->data, sizeof(wchar_t *) * array->capacity);
    }
    array->data[array->size] = (wchar_t *)malloc(sizeof(wchar_t) * (wcslen(element) + 1));
    wcscpy(array->data[array->size], element);
    array->size++;
}

void freeArray(DynamicArray *array) {
    for (int i = 0; i < array->size; i++) {
        free(array->data[i]);
    }
    free(array->data);
}

int main() {
    setlocale(LC_ALL, "uk_UA.UTF-8");
    FILE *file;
    errno_t err = fopen_s(&file, "ukr.txt", "r, ccs=UTF-8");
    if (err != 0) {
        fprintf(stderr, "Could not open file\n");
        return 1;
    }

    DynamicArray array;
    initArray(&array, 10);

    wchar_t *word;
    while ((word = get_next_word(file)) != NULL) {
        printf(L"Read word: %S\n", word);
        if (isUniqueChars(word)) {
            insertArray(&array, word);
        }
        free(word);
    }

    fclose(file);

    for (int i = 0; i < array.size; i++) {
        printf(L"%S\n", array.data[i]);
    }

    freeArray(&array);

    return 0;
}
