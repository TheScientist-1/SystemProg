#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_TOKEN_SIZE 256
#define TEST_FILE "D:\\ФКНК\\3 курс\\СисПрог\\Лабораторні\\Lab3\\test\\source.java"
#define RESULT_FILE "D:\\ФКНК\\3 курс\\СисПрог\\Лабораторні\\Lab3\\result.txt"

typedef enum {
    TOKEN_NONE,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_DIRECTIVE,
    TOKEN_COMMENT,
    TOKEN_RESERVED_WORD,
    TOKEN_OPERATOR,
    TOKEN_DELIMITER,
    TOKEN_IDENTIFIER,
    TOKEN_UNKNOWN
} TokenType;

const char* TokenTypeToString(TokenType type) {
    switch (type) {
        case TOKEN_NONE: return "NONE";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_DIRECTIVE: return "DIRECTIVE";
        case TOKEN_COMMENT: return "COMMENT";
        case TOKEN_RESERVED_WORD: return "RESERVED_WORD";
        case TOKEN_OPERATOR: return "OPERATOR";
        case TOKEN_DELIMITER: return "DELIMITER";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_UNKNOWN: return "UNKNOWN";
        default: return "ERROR"; // just in case
    }
}

typedef struct {
    TokenType type;
    char value[MAX_TOKEN_SIZE];
} Token;

bool isDelimiter(char c) {
    return strchr(";,(){}.[]", c) != NULL;
}

bool isOperator(char c) {
    return strchr("+-*/=><", c) != NULL;
}

bool isReservedWord(const char* word) {
    const char* reservedWords[] = {
        "abstract", "continue", "for", "new", "switch",
        "assert", "default", "goto", "package", "synchronized",
        "boolean", "do", "if", "private", "this",
        "break", "double", "implements", "protected", "throw",
        "byte", "else", "import", "public", "throws",
        "case", "enum", "instanceof", "return", "transient",
        "catch", "extends", "int", "short", "try",
        "char", "final", "interface", "static", "void",
        "class", "finally", "long", "strictfp", "volatile",
        "const", "float", "native", "super", "while"
    };

    for (int i = 0; i < sizeof(reservedWords) / sizeof(reservedWords[0]); i++) {
        if (strcmp(word, reservedWords[i]) == 0) {
            return true;
        }
    }
    return false;
}



bool isDirective(const char* word) {
    return word[0] == '#';
}

bool isComment(const char* input, const char** end) {
    if (input[0] == '/' && (input[1] == '/' || input[1] == '*')) {
        if (input[1] == '/') {
            *end = strchr(input, '\n');
        } else {
            *end = strstr(input, "*/");
            if (*end) *end += 2;
        }
        return true;
    }
    return false;
}

bool isNumber(const char* input) {
    return isdigit(input[0]) || (input[0] == '.' && isdigit(input[1]));
}

Token processComment(const char** input) {
    Token token;
    token.type = TOKEN_COMMENT;

    if (strncmp(*input, "//", 2) == 0) { // Single-line comment
        const char* end = strchr(*input, '\n');
        if (!end) {
            end = *input + strlen(*input); // If no newline, then till the end of the string
        }
        int length = end - *input;
        strncpy(token.value, *input, length);
        token.value[length] = '\0';
        *input = end;
    } else if (strncmp(*input, "/*", 2) == 0) { // Multi-line comment
        const char* end = strstr(*input, "*/");
        if (!end) {
            printf("Error: Unterminated comment.\n");
            token.type = TOKEN_UNKNOWN;
            *input = *input + strlen(*input); // Move to end of string to avoid infinite loop
            return token;
        }
        int length = (end - *input) + 2; // +2 to include the closing "*/"
        strncpy(token.value, *input, length);
        token.value[length] = '\0';
        *input = end + 2;
    } else {
        token.type = TOKEN_UNKNOWN; // Just in case we reached here by error
    }
    return token;
}
void initToken(Token* token) {
    token->value[0] = '\0';
    token->type = TOKEN_NONE;
}

void skipWhitespace(const char** input) {
    while (isspace(**input)) {
        (*input)++;
    }
}

Token getNextToken(const char** input) {
    Token token;
    initToken(&token);

    skipWhitespace(input);

    if (**input == '\0') {
        return token;
    }

    // Check for comments first to avoid mishandling them
    const char* end;
    if (isComment(*input, &end)) {
        token = processComment(input);
        return token;
    }

    // Check if we're starting a comment
    if (strncmp(*input, "//", 2) == 0 || strncmp(*input, "/*", 2) == 0) {
        return processComment(input);
    }

    if (isDelimiter(**input)) {
        token.type = TOKEN_DELIMITER;
        token.value[0] = **input;
        token.value[1] = '\0';
        (*input)++;
    } else if (isOperator(**input)) {
        token.type = TOKEN_OPERATOR;
        token.value[0] = **input;
        token.value[1] = '\0';
        (*input)++;
    } else if (isalpha(**input) || **input == '_') {
        int i = 0;
        while (isalnum(**input) || **input == '_') {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';

        if (isReservedWord(token.value)) {
            token.type = TOKEN_RESERVED_WORD;
        } else {
            token.type = TOKEN_IDENTIFIER;
        }
    } else if (**input == '"') { // New code to handle string literals
        int i = 0;
        token.value[i++] = **input; // starting quote
        (*input)++;
        while (**input != '"' && **input != '\0') {
            token.value[i++] = **input;
            (*input)++;
        }
        if (**input == '"') {
            token.value[i++] = **input; // closing quote
            (*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_STRING;
    } else if (isDirective(*input)) {
        token.type = TOKEN_DIRECTIVE;
        int i = 0;
        while (!isspace(**input) && **input != '\0') {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
    } else if (isComment(*input, input)) {
        token.type = TOKEN_COMMENT;
        // TODO: You can extract the whole comment if needed
    } else if (isNumber(*input)) {
        token.type = TOKEN_NUMBER;
        int i = 0;
        bool hasDot = false;
        while (isdigit(**input) || **input == '.') {
            if (**input == '.' && hasDot) {
                // Error: multiple dots in a number
                break;
            }
            if (**input == '.') {
                hasDot = true;
            }
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
    } else {
        token.type = TOKEN_UNKNOWN;
        token.value[0] = **input;
        token.value[1] = '\0';
        (*input)++;
    }

    return token;
}

char* readFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = malloc(length + 1);
    if (content == NULL) {
        printf("Failed to allocate memory for file content\n");
        fclose(file);
        return NULL;
    }

    fread(content, 1, length, file);
    content[length] = '\0'; // Null-terminate the string

    fclose(file);

    return content;
}

int main() {
    FILE* inputFile = fopen(TEST_FILE, "r");
    if (!inputFile) {
        perror("Failed to open the input file");
        return 1;
    }

    FILE* outputFile = fopen(RESULT_FILE, "w");
    if (!outputFile) {
        perror("Failed to open the output file");
        fclose(inputFile);
        return 2;
    }

    // Determine the file size
    fseek(inputFile, 0, SEEK_END);
    long fsize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    // Read the entire file into a buffer
    char* inputBuffer = (char*) malloc(fsize + 1);
    fread(inputBuffer, 1, fsize, inputFile);
    inputBuffer[fsize] = '\0';

    const char* input = inputBuffer;
    Token token;

    int tokenCount = 0; // Count the number of valid tokens
    while (*input != '\0') {
        token = getNextToken(&input);
        if (token.type != TOKEN_NONE) {
            tokenCount++;
        }
    }

    input = inputBuffer; // Reset the input pointer
    int printedTokens = 0;
    while (*input != '\0') {
        token = getNextToken(&input);
        if (token.type != TOKEN_NONE && printedTokens < tokenCount - 3) {
            fprintf(outputFile, "<%s, %s>\n", token.value, TokenTypeToString(token.type));
            printedTokens++;
        }
    }

    // Cleanup
    free(inputBuffer);
    fclose(inputFile);
    fclose(outputFile);

    return 0;
}
