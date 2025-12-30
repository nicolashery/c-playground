#ifndef SCANNER_H
#define SCANNER_H

#include "ledger.h"

#include <stddef.h>

#define MAX_TOKEN_LENGTH 256

/* ============================================================================ */
/* Token                                                                        */
/* ============================================================================ */

typedef enum {
    TOKEN_INVALID,
    TOKEN_DATE,     // 2014-01-01
    TOKEN_KEYWORD,  // commodity, open, txn
    TOKEN_ACCOUNT,  // Assets:Checking
    TOKEN_NUMBER,   // -45.12
    TOKEN_CURRENCY, // USD
    TOKEN_STRING,   // "Groceries"
    TOKEN_FLAG,     // * or !
    TOKEN_NEWLINE,  // \n
    TOKEN_INDENT,   // leading whitespace
    TOKEN_EOF,      // end of file
} TokenType;

char *token_type_to_string(TokenType type);

typedef struct {
    TokenType type;
    StringSlice text;
    size_t line;
} Token;

typedef struct {
    Token *data;
    size_t size;
    size_t capacity;
} TokenArray;

TokenArray *token_array_create(size_t capacity);
void token_array_free(TokenArray *arr);
void token_array_push(TokenArray *arr, Token value);

/* ============================================================================ */
/* Scanner                                                                      */
/* ============================================================================ */

typedef struct {
    // Ownership: Buffer is not owned by Scanner and needs to be free by caller
    const char *buffer;
    const char *current;
    size_t line;
    bool at_line_start;
} Scanner;

Scanner scanner_init(char *buffer);
Token scanner_next_token(Scanner *s);

/* ============================================================================ */
/* Test                                                                         */
/* ============================================================================ */

int test_scanner(char *ledger_path);

#endif
