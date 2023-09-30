#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>




typedef struct {
    wchar_t **data;
    int size;
    int capacity;
} DynamicArray;

bool isUniqueChars(wchar_t * str) {
    bool char_set[255] = {false};
    for (int i = 0; i < wcslen(str); i++) {
        int val = (int) str[i];
        if(char_set[val]) return false;
        char_set[val] = true;
    }
    return true;
}

void extractWords(wchar_t* line, DynamicArray* array) {
    wchar_t word[31];
    int wordIndex = 0;
    for (int i = 0; i < wcslen(line); i++) {
        if (iswalpha(line[i]) || line[i] == L'_') {
            word[wordIndex++] = line[i];
        } else if (wordIndex > 0) {
            word[wordIndex] = '\0';
            if (isUniqueChars(word)) {
                insertArray(array, word);
            }
            wordIndex = 0;
        }
    }
    if (wordIndex > 0) {
        word[wordIndex] = '\0';
        if (isUniqueChars(word)) {
            insertArray(array, word);
        }
    }
}

void initArray(DynamicArray* array, int capacity) {
    array->data = (wchar_t **)malloc(sizeof(wchar_t *) * capacity);
    array->size = 0;
    array->capacity = capacity;
}

void insertArray(DynamicArray* array, const wchar_t* element) {
    for (int i = 0; i < array->size; i++) {
        if (wcscmp(array->data[i], element) == 0) {
            return;  // Avoid duplicates
        }
    }
    if (array->size == array->capacity) {
        array->capacity *= 2;
        array->data = (wchar_t  **)realloc(array->data, sizeof(wchar_t  *) * array->capacity);
    }
    array->data[array->size] = (wchar_t *)malloc(sizeof(wchar_t) * (wcslen(element) + 1));
    wcscpy(array->data[array->size], element);
    array->size++;
    wprintf(L"Inserted word: %ls\n", element);

}

void freeArray(DynamicArray* array) {
    for (int i = 0; i < array->size; i++) {
        free(array->data[i]);
    }
    free(array->data);
}


int main() {
    setlocale(LC_ALL, "uk_UA.UTF-8");

    FILE *file = fopen("ukr.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file\n");
        return 1;
    }

    DynamicArray array;
    initArray(&array, 10);

    char line_mb[1024];
    wchar_t line[1024];
    while (fgetws(line_mb, sizeof(line_mb), file)) {
        mbstowcs(line, line_mb, sizeof(line) / sizeof(wchar_t));  // Convert multibyte to wide characters
        extractWords(line, &array);
    }

    fclose(file);

/*while (fgets(line_mb, sizeof(line_mb), file)) {
        size_t len = mbstowcs(NULL, line_mb, 0);  // Get the required length
        if(len == (size_t)-1) {
            fprintf(stderr, "Conversion error\n");
            continue;
        }

        mbstowcs(line, line_mb, len + 1);  // Convert multibyte to wide characters
        line[len] = L'\0';  // Ensure the string is null-terminated
        extractWords(line, &array);
    }*/

    for (int i = 0; i < array.size; i++) {
        wprintf(L"%ls\n", array.data[i]);
    }

    freeArray(&array);

    return 0;
    }
