#include "commands.h"

#include "ledger.h"
#include "parser.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================ */
/* Check                                                                        */
/* ============================================================================ */

int run_check(char *ledger_path) {
    Ledger *ledger = ledger_create_from_file(ledger_path);
    if (ledger == NULL) {
        printf("Error creating ledger\n");
        return EXIT_FAILURE;
    }

    bool success = true;

    success = parse_ledger(ledger);
    if (!success) {
        ledger_free(ledger);
        return EXIT_FAILURE;
    }

    success = check_ledger(ledger);

    ledger_free(ledger);

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* ============================================================================ */
/* Balance                                                                      */
/* ============================================================================ */

int run_balance(char *ledger_path) {
    Ledger *ledger = ledger_create_from_file(ledger_path);
    if (ledger == NULL) {
        printf("Error creating ledger\n");
        return EXIT_FAILURE;
    }

    bool success = true;

    success = parse_ledger(ledger);
    if (!success) {
        ledger_free(ledger);
        return EXIT_FAILURE;
    }

    success = check_ledger(ledger);
    if (!success) {
        ledger_free(ledger);
        return EXIT_FAILURE;
    }

    long *balances = calloc(ledger->accounts->size, sizeof(long));
    if (balances == NULL) {
        ledger_free(ledger);
        printf("Error allocating balances array\n");
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < ledger->postings->size; i++) {
        Posting *posting = &ledger->postings->data[i];
        balances[posting->account_index] += posting->amount.number;
    }

    long category_totals[ACCOUNT_TYPE_COUNT] = {0};
    for (size_t i = 0; i < ledger->accounts->size; i++) {
        AccountType type = ledger->accounts->data[i].type;
        category_totals[type] += balances[i];
    }

    size_t max_account_length = 0;
    for (size_t i = 0; i < ledger->accounts->size; i++) {
        StringSlice name = ledger->accounts->data[i].name;
        if (name.len > max_account_length) {
            max_account_length = name.len;
        }
    }
    // Unlikely root account name longer than all leaf accounts
    // but for good measure checking anyways
    for (AccountType type = ASSETS; type < ACCOUNT_TYPE_COUNT; type++) {
        size_t len = strlen(account_type_to_string(type));
        if (len > max_account_length) {
            max_account_length = len;
        }
    }

    typedef char BalanceString[MAX_NUMBER_LENGTH];

    BalanceString *balance_strs = calloc(ledger->accounts->size, sizeof(BalanceString));
    if (balance_strs == NULL) {
        ledger_free(ledger);
        free(balances);
        printf("Error allocating balance strings array\n");
        return EXIT_FAILURE;
    }
    size_t max_balance_length = 0;
    for (size_t i = 0; i < ledger->accounts->size; i++) {
        cents_to_cstr(balances[i], balance_strs[i], MAX_NUMBER_LENGTH);
        size_t len = strlen(balance_strs[i]);
        if (len > max_balance_length) {
            max_balance_length = len;
        }
    }

    BalanceString category_total_strs[ACCOUNT_TYPE_COUNT] = {0};
    for (AccountType type = ASSETS; type < ACCOUNT_TYPE_COUNT; type++) {
        cents_to_cstr(category_totals[type], category_total_strs[type], MAX_NUMBER_LENGTH);
        size_t len = strlen(category_total_strs[type]);
        if (len > max_balance_length) {
            max_balance_length = len;
        }
    }

    assert(ledger->currencies->size == 1 && "Expected single currency");
    char currency[MAX_TOKEN_LENGTH] = {0};
    slice_to_cstr(ledger->currencies->data[0], currency, MAX_TOKEN_LENGTH);

    for (size_t i = 0; i < ledger->accounts->size; i++) {
        char buf[MAX_TOKEN_LENGTH] = {0};
        slice_to_cstr(ledger->accounts->data[i].name, buf, MAX_TOKEN_LENGTH);
        printf("%-*s   ", (int)max_account_length, buf);
        printf("%*s", (int)max_balance_length, balance_strs[i]);
        printf(" %s\n", currency);
    }
    printf("\n");
    for (AccountType type = ASSETS; type < ACCOUNT_TYPE_COUNT; type++) {
        printf("%-*s   ", (int)max_account_length, account_type_to_string(type));
        printf("%*s", (int)max_balance_length, category_total_strs[type]);
        printf(" %s\n", currency);
    }

    free(balances);
    free(balance_strs);
    ledger_free(ledger);

    return EXIT_SUCCESS;
}
