#include "scanner.h"

#include "ledger.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/* ============================================================================ */
/* Token                                                                        */
/* ============================================================================ */

char *token_type_to_string(TokenType type) {
    switch (type) {
    case TOKEN_INVALID:
        return "INVALID";
    case TOKEN_DATE:
        return "DATE";
    case TOKEN_KEYWORD:
        return "KEYWORD";
    case TOKEN_ACCOUNT:
        return "ACCOUNT";
    case TOKEN_NUMBER:
        return "NUMBER";
    case TOKEN_CURRENCY:
        return "CURRENCY";
    case TOKEN_STRING:
        return "STRING";
    case TOKEN_FLAG:
        return "FLAG";
    case TOKEN_NEWLINE:
        return "NEWLINE";
    case TOKEN_INDENT:
        return "INDENT";
    case TOKEN_EOF:
        return "EOF";
    default:
        return "UNKNOWN";
    }
}

void print_token(Token token) {
    printf("%s", token_type_to_string(token.type));

    if (token.type == TOKEN_EOF) {
        printf("\n");
        return;
    }

    printf(" [line %zu]", token.line);

    if (token.type == TOKEN_NEWLINE) {
        printf("\n");
        return;
    }

    if (token.type == TOKEN_INDENT) {
        printf(" (size %zu)\n", token.text.len);
        return;
    }

    char token_text[MAX_TOKEN_LENGTH] = {0};
    slice_to_cstr(token.text, token_text, MAX_TOKEN_LENGTH);
    printf(" %s\n", token_text);
}

TokenArray *token_array_create(size_t capacity) {
    TokenArray *arr = malloc(sizeof(TokenArray));
    if (arr == NULL) {
        return NULL;
    }

    arr->data = malloc(capacity * sizeof(Token));
    if (arr->data == NULL) {
        free(arr);
        return NULL;
    }

    arr->size = 0;
    arr->capacity = capacity;
    return arr;
}

void token_array_free(TokenArray *arr) {
    if (arr == NULL) {
        return;
    }

    free(arr->data);
    free(arr);
}

void token_array_push(TokenArray *arr, Token value) {
    if (arr->size < arr->capacity) {
        arr->data[arr->size] = value;
        arr->size++;
        return;
    }

    size_t new_capacity = arr->capacity * 2;
    Token *new_data = realloc(arr->data, new_capacity * sizeof(Token));
    if (new_data == NULL) {
        return;
    }

    arr->data = new_data;
    arr->data[arr->size] = value;
    arr->size++;
    arr->capacity = new_capacity;
}

/* ============================================================================ */
/* Scanner                                                                      */
/* ============================================================================ */

Scanner scanner_init(char *buffer) {
    return (Scanner){
        .buffer = buffer,
        .current = buffer,
        .line = 1,
        .at_line_start = true,
    };
}

void scanner_skip_whitespace(Scanner *s) {
    char c = *s->current;

    while (c != '\0') {
        if (c == ' ' || c == '\t') {
            s->current++;
            c = *s->current;
        } else {
            break;
        }
    }
}

void scanner_skip_comments(Scanner *s) {
    char c = *s->current;

    // Consume multiple lines of consecutive comments
    while (c == ';') {
        while (c != '\n' && c != '\0') {
            s->current++;
            c = *s->current;
        }

        if (c == '\n') {
            // Consume newline
            s->current++;
            s->line++;
            s->at_line_start = true;

            // Check if next line is also a comment
            const char *start = s->current;
            scanner_skip_whitespace(s);
            c = *s->current;
            if (c != ';') {
                // Backtrack if not
                s->current = start;
                c = *s->current;
            }
        }
    }
}

bool try_scan_date(Scanner *s, Token *t) {
    const char *start = s->current;
    const char *peek = s->current;

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
    t->text = (StringSlice){
        .start = start,
        .len = (size_t)(peek - start),
    };
    t->line = s->line;

    s->current = peek;

    return true;
}

bool try_scan_number(Scanner *s, Token *t) {
    const char *start = s->current;
    const char *peek = s->current;

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
    while (*peek != '\0' && isdigit(*peek)) {
        peek++;
    }

    // Optional decimal point (.)
    if (*peek == '.') {
        peek++;
        // At least one digit after decimal point
        if (!isdigit(*peek)) {
            return false;
        }
        peek++;
        // Consume rest of digits after decimal point
        while (*peek != '\0' && isdigit(*peek)) {
            peek++;
        }
    }

    t->type = TOKEN_NUMBER;
    t->text = (StringSlice){
        .start = start,
        .len = (size_t)(peek - start),
    };
    t->line = s->line;

    s->current = peek;

    return true;
}

Token scanner_next_token(Scanner *s) {
    char c;
    Token t = {0};

    // Always skip comments first
    scanner_skip_comments(s);
    if (!s->at_line_start) {
        // If not at line start, skip inter-token whitespace
        // (this might reveal end-of-line comment)
        scanner_skip_whitespace(s);
        c = *s->current;
        if (c == ';') {
            // Consume only end of line comment and fall through to emit new line token
            while (c != '\n' && c != '\0') {
                s->current++;
                c = *s->current;
            }
        }
    } else {
        // If at line start, check if indented comment line
        const char *start = s->current;
        scanner_skip_whitespace(s);
        c = *s->current;
        if (c == ';') {
            // If yes, skip comments
            scanner_skip_comments(s);
        } else {
            // If not, backtrack
            s->current = start;
        }
    }

    // Check line start for indent after skipping all comments as
    // skipping comments might place us on a new line start
    if (s->at_line_start) {
        // Check if any leading whitespace followed by regular character,
        // and if yes emit indent token
        const char *start = s->current;
        scanner_skip_whitespace(s);
        c = *s->current;
        if (s->current != start && c != '\0' && c != '\n') {
            t.type = TOKEN_INDENT;
            t.text = (StringSlice){
                .start = start,
                .len = (size_t)(s->current - start),
            };
            t.line = s->line;

            s->at_line_start = false;

            return t;
        }

        // If not fall through to rest of scanner
        // and make sure to flag line start as already checked
        s->at_line_start = false;
    }

    c = *s->current;

    // EOF
    if (c == '\0') {
        t.type = TOKEN_EOF;
        t.line = s->line;
        return t;
    }

    // Newline
    if (c == '\n') {
        t.type = TOKEN_NEWLINE;
        t.text = (StringSlice){
            .start = s->current,
            .len = 1,
        };
        t.line = s->line;
        s->current++;
        s->line++;
        s->at_line_start = true;
        return t;
    }

    // Flag
    if (c == '*' || c == '!') {
        t.type = TOKEN_FLAG;
        t.text = (StringSlice){
            .start = s->current,
            .len = 1,
        };
        t.line = s->line;
        s->current++;
        return t;
    }

    // Keyword
    if (islower(c)) {
        const char *start = s->current;
        while (islower(c)) {
            s->current++;
            c = *s->current;
        }

        t.type = TOKEN_KEYWORD;
        t.text = (StringSlice){
            .start = start,
            .len = (size_t)(s->current - start),
        };
        t.line = s->line;
        return t;
    }

    // Currency or Account
    if (isupper(c)) {
        const char *start = s->current;
        bool all_uppercase = true;
        bool has_colon = false;
        while (isalnum(c) || c == ':') {
            if (islower(c)) {
                all_uppercase = false;
            } else if (c == ':') {
                has_colon = true;
            }

            s->current++;
            c = *s->current;
        }

        if (has_colon) {
            t.type = TOKEN_ACCOUNT;
        } else if (all_uppercase) {
            t.type = TOKEN_CURRENCY;
        } else {
            t.type = TOKEN_INVALID;
        }

        t.text = (StringSlice){
            .start = start,
            .len = (size_t)(s->current - start),
        };
        t.line = s->line;
        return t;
    }

    // String
    if (c == '\"') {
        const char *start = s->current;
        bool invalid = false;
        s->current++;
        c = *s->current;
        while (c != '\"') {
            if (c == '\0' || c == '\n') {
                invalid = true;
                break;
            }

            s->current++;
            c = *s->current;
        }

        if (invalid) {
            t.type = TOKEN_INVALID;
            t.text = (StringSlice){
                .start = start,
                .len = 1,
            };
            t.line = s->line;
            // Go back to just after the opening '\"'
            s->current = start + 1;
            return t;
        }

        // Consume closing '\"'
        s->current++;

        t.type = TOKEN_STRING;
        t.text = (StringSlice){
            .start = start,
            .len = (size_t)(s->current - start),
        };
        t.line = s->line;
        return t;
    }

    // Date or Number
    if (isdigit(c)) {
        if (try_scan_date(s, &t)) {
            return t;
        }

        if (try_scan_number(s, &t)) {
            return t;
        }
    } else if (c == '-') {
        if (try_scan_number(s, &t)) {
            return t;
        }
    }

    t.type = TOKEN_INVALID;
    t.text = (StringSlice){
        .start = s->current,
        .len = 1,
    };
    t.line = s->line;
    s->current++;

    return t;
}

/* ============================================================================ */
/* Test                                                                         */
/* ============================================================================ */

int test_scanner(char *ledger_path) {
    char *file_buffer = read_file(ledger_path);
    if (file_buffer == NULL) {
        printf("Error reading file\n");
        return EXIT_FAILURE;
    }

    Scanner scanner = scanner_init(file_buffer);
    Token token = {0};
    for (;;) {
        token = scanner_next_token(&scanner);
        print_token(token);

        if (token.type == TOKEN_EOF) {
            break;
        }
    }

    free(file_buffer);

    return EXIT_SUCCESS;
}
