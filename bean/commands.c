#include "commands.h"

#include "ledger.h"
#include "parser.h"
#include "scanner.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================ */
/* Check                                                                        */
/* ============================================================================ */

int run_check(char *ledger_path) {
    int result = EXIT_SUCCESS;
    Ledger *ledger = NULL;

    ledger = ledger_create_from_file(ledger_path);
    if (ledger == NULL) {
        printf("Error creating ledger\n");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    if (!parse_ledger(ledger)) {
        result = EXIT_FAILURE;
        goto cleanup;
    }

    if (!check_ledger(ledger)) {
        result = EXIT_FAILURE;
        goto cleanup;
    }

cleanup:
    ledger_free(ledger);
    return result;
}

/* ============================================================================ */
/* Balance                                                                      */
/* ============================================================================ */

typedef char BalanceString[MAX_NUMBER_LENGTH];

int run_balance(char *ledger_path) {
    int result = EXIT_SUCCESS;
    Ledger *ledger = NULL;
    long *balances = NULL;
    BalanceString *balance_strs = NULL;

    ledger = ledger_create_from_file(ledger_path);
    if (ledger == NULL) {
        printf("Error creating ledger\n");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    if (!parse_ledger(ledger)) {
        result = EXIT_FAILURE;
        goto cleanup;
    }

    if (!check_ledger(ledger)) {
        result = EXIT_FAILURE;
        goto cleanup;
    }

    balances = calloc(ledger->accounts->size, sizeof(long));
    if (balances == NULL) {
        printf("Error allocating balances array\n");
        result = EXIT_FAILURE;
        goto cleanup;
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

    balance_strs = calloc(ledger->accounts->size, sizeof(BalanceString));
    if (balance_strs == NULL) {
        printf("Error allocating balance strings array\n");
        result = EXIT_FAILURE;
        goto cleanup;
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

cleanup:
    free(balance_strs);
    free(balances);
    ledger_free(ledger);
    return result;
}
