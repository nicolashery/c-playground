#ifndef PARSER_H
#define PARSER_H

#include "ledger.h"
#include "scanner.h"

#define MAX_ERROR_MESSAGE_LENGTH 256
#define MAX_NUMBER_LENGTH 16

/* ============================================================================ */
/* Parser                                                                       */
/* ============================================================================ */

typedef struct {
    // Ownership: Token array is not owned by Parser and needs to be freed by caller
    TokenArray *tokens;
    size_t current;
    Ledger *ledger;
    bool is_eof;
    bool has_error;
    char error_message[MAX_ERROR_MESSAGE_LENGTH];
    size_t error_line;
} Parser;

Parser parser_init(TokenArray *tokens, Ledger *ledger);
bool parse_ledger(Ledger *ledger);

/* ============================================================================ */
/* Test                                                                         */
/* ============================================================================ */

int test_parser(char *ledger_path);

#endif
