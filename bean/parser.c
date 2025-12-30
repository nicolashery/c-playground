#include "parser.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN_ARRAY_INITIAL_CAPACITY 100

/* ============================================================================ */
/* Parser                                                                       */
/* ============================================================================ */

Parser parser_init(TokenArray *tokens, Ledger *ledger) {
    assert(tokens->data[tokens->size - 1].type == TOKEN_EOF && "Tokens must end with EOF");

    Parser parser = {0};

    parser.tokens = tokens;
    parser.ledger = ledger;

    return parser;
}

Token *parser_peek(Parser *p) {
    return &p->tokens->data[p->current];
}

Token *parser_lookahead(Parser *p) {
    size_t next = p->current + 1;
    if (next >= p->tokens->size) {
        return &p->tokens->data[p->current];
    }

    return &p->tokens->data[next];
}

void parser_advance(Parser *p) {
    size_t next = p->current + 1;
    if (next >= p->tokens->size) {
        return;
    }

    p->current = next;
}

Token *parser_match(Parser *p, TokenType token_type) {
    Token *t = parser_peek(p);
    if (t->type != token_type) {
        return NULL;
    }

    parser_advance(p);
    return t;
}

Token *parser_expect(Parser *p, TokenType token_type) {
    Token *t = parser_match(p, token_type);
    if (t == NULL) {
        Token *current = parser_peek(p);
        p->has_error = true;
        (void)snprintf(p->error_message,
                       MAX_ERROR_MESSAGE_LENGTH,
                       "Expected token %s got %s",
                       token_type_to_string(token_type),
                       token_type_to_string(current->type));
        p->error_line = current->line;
        return NULL;
    }

    return t;
}

Date parse_date(StringSlice text) {
    assert(text.len == 10 && "Date string must be of length 10");

    char buf[11];
    memcpy(buf, text.start, 10);
    buf[10] = '\0';

    long year = strtol(buf, NULL, 10);
    long month = strtol(buf + 5, NULL, 10);
    long day = strtol(buf + 8, NULL, 10);

    return (Date){
        .year = (int)year,
        .month = (int)month,
        .day = (int)day,
    };
}

void parse_commodity_directive(Parser *p) {
    Token *t;

    // Parse date
    t = parser_expect(p, TOKEN_DATE);
    if (p->has_error) {
        return;
    }
    (void)parse_date(t->text);

    // Parse "commodity" keyword
    t = parser_expect(p, TOKEN_KEYWORD);
    if (p->has_error) {
        return;
    }
    StringSlice keyword = t->text;
    if (!slice_equals_cstr(keyword, "commodity")) {
        p->has_error = true;
        char buf[MAX_TOKEN_LENGTH] = {0};
        slice_to_cstr(keyword, buf, MAX_TOKEN_LENGTH);
        (void)snprintf(p->error_message,
                       MAX_ERROR_MESSAGE_LENGTH,
                       "Expected keyword 'commodity' got '%s'",
                       buf);
        p->error_line = t->line;
        return;
    };

    // Parse currency
    t = parser_expect(p, TOKEN_CURRENCY);
    if (p->has_error) {
        return;
    }
    StringSlice currency = t->text;
    Ledger *l = p->ledger;
    bool currency_already_exists = false;
    for (size_t i = 0; i < l->currencies->size; i++) {
        if (slice_equals(l->currencies->data[i], currency)) {
            currency_already_exists = true;
            break;
        }
    }
    if (currency_already_exists) {
        p->has_error = true;
        char buf[MAX_TOKEN_LENGTH] = {0};
        slice_to_cstr(currency, buf, MAX_TOKEN_LENGTH);
        (void)snprintf(
            p->error_message, MAX_ERROR_MESSAGE_LENGTH, "Commodity '%s' already declared", buf);
        p->error_line = t->line;
        return;
    }

    t = parser_match(p, TOKEN_NEWLINE);
    if (t == NULL) {
        t = parser_match(p, TOKEN_EOF);
    }
    if (t == NULL) {
        p->has_error = true;
        (void)snprintf(p->error_message,
                       MAX_ERROR_MESSAGE_LENGTH,
                       "Expected newline or EOF after commodity directive");
        p->error_line = t->line;
        return;
    }

    string_slice_array_push(l->currencies, currency);
}

void parse_open_directive(Parser *p) {
    Token *t;

    // Parse date
    t = parser_expect(p, TOKEN_DATE);
    if (p->has_error) {
        return;
    }
    (void)parse_date(t->text);

    // Parse "open" keyword
    t = parser_expect(p, TOKEN_KEYWORD);
    if (p->has_error) {
        return;
    }
    StringSlice keyword = t->text;
    if (!slice_equals_cstr(keyword, "open")) {
        p->has_error = true;
        char buf[MAX_TOKEN_LENGTH] = {0};
        slice_to_cstr(keyword, buf, MAX_TOKEN_LENGTH);
        (void)snprintf(
            p->error_message, MAX_ERROR_MESSAGE_LENGTH, "Expected keyword 'open' got '%s'", buf);
        p->error_line = t->line;
        return;
    };

    // Parse account
    t = parser_expect(p, TOKEN_ACCOUNT);
    if (p->has_error) {
        return;
    }
    StringSlice account_name = t->text;
    StringSlice account_root = account_extract_root(account_name);
    AccountType account_type = account_extract_type(account_root);
    if (account_type == ACCOUNT_TYPE_INVALID) {
        p->has_error = true;
        char buf[MAX_TOKEN_LENGTH] = {0};
        slice_to_cstr(account_root, buf, MAX_TOKEN_LENGTH);
        (void)snprintf(
            p->error_message, MAX_ERROR_MESSAGE_LENGTH, "Invalid account type '%s'", buf);
        p->error_line = t->line;
        return;
    }
    Ledger *l = p->ledger;
    bool account_already_exists = false;
    for (size_t i = 0; i < l->accounts->size; i++) {
        if (slice_equals(l->accounts->data[i].name, account_name)) {
            account_already_exists = true;
            break;
        }
    }
    if (account_already_exists) {
        p->has_error = true;
        char buf[MAX_TOKEN_LENGTH] = {0};
        slice_to_cstr(account_name, buf, MAX_TOKEN_LENGTH);
        (void)snprintf(p->error_message,
                       MAX_ERROR_MESSAGE_LENGTH,
                       "Account with name '%s' already opened",
                       buf);
        p->error_line = t->line;
        return;
    }
    Account account = {
        .type = account_type,
        .name = account_name,
    };

    t = parser_match(p, TOKEN_NEWLINE);
    if (t == NULL) {
        t = parser_match(p, TOKEN_EOF);
    }
    if (t == NULL) {
        p->has_error = true;
        (void)snprintf(p->error_message,
                       MAX_ERROR_MESSAGE_LENGTH,
                       "Expected newline or EOF after open directive");
        p->error_line = t->line;
        return;
    }

    account_array_push(l->accounts, account);
}

int find_account(AccountArray *accounts, StringSlice account_name) {
    int result = -1;
    for (size_t i = 0; i < accounts->size; i++) {
        if (slice_equals(accounts->data[i].name, account_name)) {
            result = (int)i;
            break;
        }
    }
    return result;
}

int find_currency(StringSliceArray *currencies, StringSlice currency) {
    int result = -1;
    for (size_t i = 0; i < currencies->size; i++) {
        if (slice_equals(currencies->data[i], currency)) {
            result = (int)i;
            break;
        }
    }
    return result;
}

long parse_amount_number(Parser *p, Token *t) {
    StringSlice text = t->text;
    assert(text.len < MAX_NUMBER_LENGTH && "Number string too long");

    // Copy over text omitting '.' to convert to cents
    char buf[MAX_NUMBER_LENGTH];
    size_t j = 0;
    size_t decimal_index = 0;
    size_t decimal_count = 0;
    for (size_t i = 0; i < text.len; i++) {
        if (text.start[i] == '.') {
            decimal_index = i;
            break;
        }

        buf[j] = text.start[i];
        j++;
    }
    for (size_t i = decimal_index; i < text.len; i++) {
        if (text.start[i] == '.') {
            continue;
        }

        decimal_count++;
        buf[j] = text.start[i];
        j++;
    }
    buf[j] = '\0';

    if (decimal_count != AMOUNT_DECIMAL_PLACES) {
        p->has_error = true;
        (void)snprintf(p->error_message,
                       MAX_ERROR_MESSAGE_LENGTH,
                       "Expected exacly %d decimal places",
                       AMOUNT_DECIMAL_PLACES);
        p->error_line = t->line;
        return 0;
    }

    return strtol(buf, NULL, 10);
}

bool parse_posting(Parser *p, Posting *posting) {
    Token *t;

    t = parser_match(p, TOKEN_NEWLINE);
    if (t != NULL) {
        return false;
    }

    t = parser_match(p, TOKEN_EOF);
    if (t != NULL) {
        return false;
    }

    (void)parser_expect(p, TOKEN_INDENT);
    if (p->has_error) {
        return false;
    }

    t = parser_expect(p, TOKEN_ACCOUNT);
    if (p->has_error) {
        return false;
    }
    StringSlice account_name = t->text;
    int account_index = find_account(p->ledger->accounts, account_name);
    if (account_index < 0) {
        p->has_error = true;
        char buf[MAX_TOKEN_LENGTH] = {0};
        slice_to_cstr(account_name, buf, MAX_TOKEN_LENGTH);
        (void)snprintf(
            p->error_message, MAX_ERROR_MESSAGE_LENGTH, "Account with name '%s' never opened", buf);
        p->error_line = t->line;
        return false;
    }
    posting->account_index = (size_t)account_index;

    t = parser_expect(p, TOKEN_NUMBER);
    if (p->has_error) {
        return false;
    }
    Amount amount = {0};
    long number = parse_amount_number(p, t);
    if (p->has_error) {
        return false;
    }
    amount.number = number;

    t = parser_expect(p, TOKEN_CURRENCY);
    if (p->has_error) {
        return false;
    }
    StringSlice currency = t->text;
    int currency_index = find_currency(p->ledger->currencies, currency);
    if (currency_index < 0) {
        p->has_error = true;
        char buf[MAX_TOKEN_LENGTH] = {0};
        slice_to_cstr(currency, buf, MAX_TOKEN_LENGTH);
        (void)snprintf(p->error_message,
                       MAX_ERROR_MESSAGE_LENGTH,
                       "Commodity with name '%s' never declared",
                       buf);
        p->error_line = t->line;
        return false;
    }
    amount.currency_index = (size_t)currency_index;

    posting->amount = amount;

    (void)parser_expect(p, TOKEN_NEWLINE);
    return !p->has_error;
}

void parse_transaction(Parser *p) {
    Token *t;

    // Parse date
    t = parser_expect(p, TOKEN_DATE);
    if (p->has_error) {
        return;
    }
    Date date = parse_date(t->text);
    size_t line = t->line;

    // Parse "txn" keyword, or a flag
    Flag flag = FLAG_NONE;
    t = parser_match(p, TOKEN_KEYWORD);
    if (t != NULL) {
        StringSlice keyword = t->text;
        if (!slice_equals_cstr(keyword, "txn")) {
            p->has_error = true;
            char buf[MAX_TOKEN_LENGTH] = {0};
            slice_to_cstr(keyword, buf, MAX_TOKEN_LENGTH);
            (void)snprintf(
                p->error_message, MAX_ERROR_MESSAGE_LENGTH, "Expected keyword 'txn' got '%s'", buf);
            p->error_line = t->line;
            return;
        };

    } else {
        t = parser_match(p, TOKEN_FLAG);
        if (t != NULL) {
            char c = t->text.start[0];
            if (c == '*') {
                flag = FLAG_OKAY;
            } else if (c == '!') {
                flag = FLAG_WARNING;
            }
        } else {
            p->has_error = true;
            char buf[MAX_TOKEN_LENGTH] = {0};
            slice_to_cstr(t->text, buf, MAX_TOKEN_LENGTH);
            (void)snprintf(p->error_message,
                           MAX_ERROR_MESSAGE_LENGTH,
                           "Expected keyword 'txn' or flag ('*' or '!') but got %s '%s'",
                           token_type_to_string(t->type),
                           buf);
            p->error_line = t->line;
            return;
        }
    }

    // Parse optional payee and/or narration
    t = parser_expect(p, TOKEN_STRING);
    if (p->has_error) {
        return;
    }
    StringSlice payee = {0};
    StringSlice narration = t->text;
    t = parser_match(p, TOKEN_STRING);
    if (t != NULL) {
        payee = narration;
        narration = t->text;
    }

    t = parser_match(p, TOKEN_NEWLINE);
    if (t == NULL) {
        t = parser_match(p, TOKEN_EOF);
    }
    if (t == NULL) {
        p->has_error = true;
        (void)snprintf(p->error_message,
                       MAX_ERROR_MESSAGE_LENGTH,
                       "Expected newline or EOF after transaction directive");
        p->error_line = t->line;
        return;
    }

    Ledger *l = p->ledger;

    Transaction transaction = {0};
    transaction.date = date;
    transaction.flag = flag;
    transaction.payee = payee;
    transaction.narration = narration;

    transaction.postings_start_index = l->postings->size;
    transaction.postings_count = 0;
    Posting posting = {0};
    while (parse_posting(p, &posting)) {
        posting_array_push(l->postings, posting);
        transaction.postings_count++;
    }

    transaction.line = line;

    if (p->has_error) {
        return;
    }

    transaction_array_push(l->transactions, transaction);
}

void parser_print_error(Parser *p) {
    if (!p->has_error) {
        return;
    }

    printf("Parse error: %s (line %zu)\n", p->error_message, p->error_line);
}

void parse_next(Parser *p) {
    Token *t = parser_peek(p);
    if (t->type == TOKEN_EOF) {
        p->is_eof = true;
        return;
    }

    if (t->type == TOKEN_INVALID) {
        p->has_error = true;
        char buf[MAX_TOKEN_LENGTH] = {0};
        slice_to_cstr(t->text, buf, MAX_TOKEN_LENGTH);
        (void)snprintf(p->error_message, MAX_ERROR_MESSAGE_LENGTH, "Invalid token '%s'", buf);
        p->error_line = t->line;
        return;
    }

    if (t->type == TOKEN_DATE) {
        Token *next = parser_lookahead(p);
        if (next->type == TOKEN_KEYWORD) {
            if (slice_equals_cstr(next->text, "commodity")) {
                parse_commodity_directive(p);
                return;
            }

            if (slice_equals_cstr(next->text, "open")) {
                parse_open_directive(p);
                return;
            }

            if (slice_equals_cstr(next->text, "txn")) {
                parse_transaction(p);
                return;
            }

            p->has_error = true;
            char buf[MAX_TOKEN_LENGTH] = {0};
            slice_to_cstr(next->text, buf, MAX_TOKEN_LENGTH);
            (void)snprintf(
                p->error_message, MAX_ERROR_MESSAGE_LENGTH, "Unrecognized keyword '%s'", buf);
            p->error_line = next->line;
            return;
        }

        if (next->type == TOKEN_FLAG) {
            parse_transaction(p);
            return;
        }

        p->has_error = true;
        (void)snprintf(p->error_message,
                       MAX_ERROR_MESSAGE_LENGTH,
                       "Expected %s or %s after %s, got %s",
                       token_type_to_string(TOKEN_KEYWORD),
                       token_type_to_string(TOKEN_FLAG),
                       token_type_to_string(TOKEN_DATE),
                       token_type_to_string(next->type));
        p->error_line = next->line;
        return;
    }

    if (t->type == TOKEN_NEWLINE) {
        parser_advance(p);
        return;
    }

    p->has_error = true;
    char buf[MAX_TOKEN_LENGTH] = {0};
    slice_to_cstr(t->text, buf, MAX_TOKEN_LENGTH);
    (void)snprintf(p->error_message,
                   MAX_ERROR_MESSAGE_LENGTH,
                   "Unexpected token %s '%s'",
                   token_type_to_string(t->type),
                   buf);
    p->error_line = t->line;
}

bool parse_ledger(Ledger *ledger) {
    TokenArray *tokens = token_array_create(TOKEN_ARRAY_INITIAL_CAPACITY);
    if (tokens == NULL) {
        printf("Error allocating token array\n");
        return false;
    }

    Scanner scanner = scanner_init(ledger->file_buffer);
    Token token = {0};
    for (;;) {
        token = scanner_next_token(&scanner);
        token_array_push(tokens, token);

        if (token.type == TOKEN_EOF) {
            break;
        }
    }

    Parser parser = parser_init(tokens, ledger);
    for (;;) {
        parse_next(&parser);

        if (parser.is_eof) {
            break;
        }

        if (parser.has_error) {
            break;
        }
    }

    bool success = true;
    if (parser.has_error) {
        parser_print_error(&parser);
        success = false;
    }

    token_array_free(tokens);
    return success;
}

/* ============================================================================ */
/* Test                                                                         */
/* ============================================================================ */

int test_parser(char *ledger_path) {
    Ledger *ledger = ledger_create_from_file(ledger_path);
    if (ledger == NULL) {
        printf("Error creating ledger\n");
        return EXIT_FAILURE;
    }

    bool success = parse_ledger(ledger);
    if (success) {
        print_ledger(ledger);
    }

    ledger_free(ledger);

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
