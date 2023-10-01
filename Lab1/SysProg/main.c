#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>
#include <locale.h>

typedef struct {
    wchar_t **data;
    int size;
    int capacity;
} DynamicArray;

bool isUniqueChars(wchar_t* str) {
    bool char_set[65536] = { false };  // Assuming a maximum of 65536 different characters
    for (int i = 0; i < wcslen(str); i++) {
        int val = str[i];
        if (char_set[val]) {
            return false;
        }
        char_set[val] = true;
    }
    return true;
}

void initArray(DynamicArray* array, int capacity) {
    array->data = (wchar_t **)malloc(sizeof(wchar_t *) * capacity);
    array->size = 0;
    array->capacity = capacity;
}

void insertArray(DynamicArray* array, wchar_t* element) {
    if (array->size == array->capacity) {
        array->capacity *= 2;
        array->data = (wchar_t **)realloc(array->data, sizeof(wchar_t *) * array->capacity);
    }
    array->data[array->size] = (wchar_t *)malloc(sizeof(wchar_t) * (wcslen(element) + 1));
    wcscpy(array->data[array->size], element);
    array->size++;
}

void freeArray(DynamicArray* array) {
    for (int i = 0; i < array->size; i++) {
        free(array->data[i]);
    }
    free(array->data);
}

void extractWords(wchar_t* line, DynamicArray* array) {
    wchar_t word[31];
    int wordIndex = 0;
    for (int i = 0; i < wcslen(line); i++) {
        if (iswalpha(line[i])) {
            word[wordIndex++] = line[i];
        } else if (wordIndex > 0) {
            word[wordIndex] = L'\0';
            if (isUniqueChars(word)) {
                insertArray(array, word);
            }
            wordIndex = 0;
        }
    }
    if (wordIndex > 0) {
        word[wordIndex] = L'\0';
        if (isUniqueChars(word)) {
            insertArray(array, word);
        }
    }
}

int main() {
    setlocale(LC_ALL, "");
    FILE *file = _wfopen(L"test_input/en.txt", L"r, ccs=UTF-8");
    if (file == NULL) {
        fwprintf(stderr, L"Could not open input file\n");
        return 1;
    }

    DynamicArray array;
    initArray(&array, 10);

    wchar_t line[1024];
    bool file_was_empty = true;
    while (fgetws(line, sizeof(line)/sizeof(wchar_t), file)) {
        file_was_empty = false;
        extractWords(line, &array);
    }

    fclose(file);

    for (int i = 0; i < array.size; i++) {
        wprintf(L"%ls\n", array.data[i]);
    }

    FILE *outfile = _wfopen(L"test_output/output.txt", L"w, ccs=UTF-8");
    if (outfile == NULL) {
        fwprintf(stderr, L"Could not open output file\n");
        freeArray(&array);
        return 1;
    }

    if (file_was_empty) {
        fwprintf(outfile, L"File was empty.\n");
        wprintf(L"File was empty.\n");
    } else if (array.size == 0) {
        fwprintf(outfile, L"No words met the specified condition.\n");
        wprintf(L"No words met the specified condition.\n");
    } else {
        for (int i = 0; i < array.size; i++) {
            fwprintf(outfile, L"%ls\n", array.data[i]);
        }
    }

    fclose(outfile);

    freeArray(&array);

    return 0;
}
