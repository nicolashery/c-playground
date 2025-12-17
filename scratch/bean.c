#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TOKEN_LENGTH 256

typedef enum {
    TOKEN_INVALID,
    TOKEN_DATE,
    TOKEN_NUMBER,
    TOKEN_DIRECTIVE,
    TOKEN_EOF,
} TokenType;

const char *token_type_names[5] = {
    [TOKEN_INVALID] = "INVALID",
    [TOKEN_DATE] = "DATE",
    [TOKEN_NUMBER] = "TOKEN_NUMBER",
    [TOKEN_DIRECTIVE] = "DIRECTIVE",
    [TOKEN_EOF] = "EOF",
};

typedef struct {
    TokenType type;
    char *start;
    size_t length;
    size_t line;
} Token;

void debug_print_token(Token token) {
    char token_text[MAX_TOKEN_LENGTH] = {};
    (void)snprintf(token_text, MAX_TOKEN_LENGTH, "%.*s", (int)token.length, token.start);
    printf("[%ld] %s \"%s\"\n", token.line, token_type_names[token.type], token_text);
}

typedef struct {
    char *start;
    size_t size;
    char *current;
    size_t line;
} Scanner;

Scanner scanner_create(char *buffer, size_t size) {
    return (Scanner){
        .start = buffer,
        .size = size,
        .current = buffer,
        .line = 1,
    };
}

bool is_eof(Scanner *s, const char *p) {
    return (*p == '\0' || p >= (s->start + s->size)) != 0;
}

bool is_whitespace(char c) {
    return isspace(c) != 0;
}

bool is_newline(char c) {
    return (c == '\n');
}

void scanner_skip_whitespace(Scanner *s) {
    while (!is_eof(s, s->current)) {
        char c = *s->current;
        if (is_newline(c)) {
            s->line++;
        }

        if (is_whitespace(c)) {
            s->current++;
            continue;
        }

        break;
    }
}

bool token_date(Scanner *s, Token *t) {
    char *peek = s->current;
    // Dates can't start with 0
    if (*peek == '0') {
        return false;
    }
    // Consume year (YYYY)
    for (int i = 0; i < 3; ++i) {
        peek++;
        if (!isdigit(*peek)) {
            return false;
        }
    }
    // Consume separator (-)
    peek++;
    if (*peek != '-') {
        return false;
    }
    // Consume month (MM)
    for (int i = 0; i < 2; ++i) {
        peek++;
        if (!isdigit(*peek)) {
            return false;
        }
    }
    // Consume separator (-)
    peek++;
    if (*peek != '-') {
        return false;
    }
    // Consume day (DD)
    for (int i = 0; i < 2; ++i) {
        peek++;
        if (!isdigit(*peek)) {
            return false;
        }
    }
    peek++;
    t->type = TOKEN_DATE;
    t->length = (size_t)(peek - t->start);

    s->current = peek;

    return true;
}

bool token_number(Scanner *s, Token *t) {
    char *peek = s->current;

    // Consume optional negative sign (-)
    if (*peek == '-') {
        peek++;
    }

    // At least one digit
    if (!isdigit(*peek)) {
        return false;
    }
    peek++;

    // Consume rest of digits
    while (!is_eof(s, peek) && isdigit(*peek)) {
        peek++;
    }

    // Optional decimal point (.)
    if (!is_eof(s, peek) && *peek == '.') {
        peek++;
        // At least one digit after decimal point
        if (!isdigit(*peek)) {
            return false;
        }
        peek++;
        // Consume rest of digits after decimal point
        while (!is_eof(s, peek) && isdigit(*peek)) {
            peek++;
        }
    }

    t->type = TOKEN_NUMBER;
    t->length = (size_t)(peek - t->start);

    s->current = peek;

    return true;
}

bool token_next(Scanner *s, Token *t) {
    scanner_skip_whitespace(s);

    t->start = s->current;
    t->length = 0;
    t->line = s->line;

    // End-of-file
    if (is_eof(s, s->current)) {
        t->type = TOKEN_EOF;
        return false;
    }

    char c = *s->current;
    if (isdigit(c)) {
        if (token_date(s, t)) {
            return true;
        }

        if (token_number(s, t)) {
            return true;
        }

        t->type = TOKEN_INVALID;
        return false;
    }

    if (c == '-') {
        if (token_number(s, t)) {
            return true;
        }

        t->type = TOKEN_INVALID;
        return false;
    }

    // TODO: other tokens
    s->current++;
    return true;
}

int bean_main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: bean <ledger_path>\n");
        return EXIT_FAILURE;
    }

    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    char *ledger_path = argv[1];
    FILE *ledger_file = fopen(ledger_path, "rb");
    if (ledger_file == NULL) {
        printf("Error opening file: %s\n", ledger_path);
        return EXIT_FAILURE;
    }

    if (fseek(ledger_file, 0, SEEK_END) != 0) {
        printf("Error seeking to end of file: %s\n", ledger_path);
        (void)fclose(ledger_file);
        return EXIT_FAILURE;
    }
    long fledger_size = ftell(ledger_file);
    if (fledger_size < 0) {
        printf("Error reading size of file: %s\n", ledger_path);
        (void)fclose(ledger_file);
        return EXIT_FAILURE;
    }
    size_t ledger_size = (size_t)fledger_size;
    if (fseek(ledger_file, 0, SEEK_SET) != 0) {
        printf("Error seeking to beginning of file: %s\n", ledger_path);
        (void)fclose(ledger_file);
        return EXIT_FAILURE;
    }
    printf("[debug] Ledger file size (bytes): %ld\n", ledger_size);

    size_t buffer_capacity = ledger_size + 1;
    char *ledger_buffer = malloc(buffer_capacity);
    if (!ledger_buffer) {
        printf("Error allocating memory for reading ledger (bytes): %ld\n", buffer_capacity);
        (void)fclose(ledger_file);
        return EXIT_FAILURE;
    }

    size_t bytes_read = fread(ledger_buffer, 1, ledger_size, ledger_file);
    if (bytes_read != ledger_size) {
        if (feof(ledger_file)) {
            printf("Error reached end-of-file before reading %ld bytes from "
                   "file: %s\n",
                   ledger_size,
                   ledger_path);
        } else if (ferror(ledger_file)) {
            printf("Error reading file: %s\n", ledger_path);
        }

        free(ledger_buffer);
        (void)fclose(ledger_file);
        return EXIT_FAILURE;
    }
    // buffer_capacity = ledger_size + 1, so this is safe
    // NOLINTNEXTLINE(clang-analyzer-security.ArrayBound)
    ledger_buffer[ledger_size] = '\0';

    int line_count = 0;
    for (size_t i = 0; i < ledger_size; ++i) {
        if (ledger_buffer[i] == '\n') {
            line_count++;
        }
    }
    printf("[debug] Ledger file size (lines): %d\n", line_count);

    Scanner scanner = scanner_create(ledger_buffer, ledger_size);
    for (;;) {
        Token token = {0};
        if (!token_next(&scanner, &token)) {
            break;
        }
        debug_print_token(token);
    }

    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed_ms = (double)(end_time.tv_sec - start_time.tv_sec) * 1.0e3 +
                        (double)(end_time.tv_nsec - start_time.tv_nsec) / 1.0e6;
    printf("[debug] Processing took: %.3f ms\n", elapsed_ms);

    free(ledger_buffer);
    if (fclose(ledger_file) != 0) {
        printf("Error closing file\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
