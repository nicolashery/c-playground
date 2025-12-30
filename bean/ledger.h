#ifndef LEDGER_H
#define LEDGER_H

#include <stddef.h>

#define AMOUNT_DECIMAL_PLACES 2

/* ============================================================================ */
/* String                                                                       */
/* ============================================================================ */

typedef struct {
    // Pointer into file buffer
    const char *start;
    // Length in bytes
    size_t len;
} StringSlice;

void slice_to_cstr(StringSlice slice, char *buffer, size_t buffer_size);
bool slice_equals_cstr(StringSlice slice, char *str);
bool slice_equals(StringSlice s1, StringSlice s2);

typedef struct {
    StringSlice *data;
    size_t size;
    size_t capacity;
} StringSliceArray;

StringSliceArray *string_slice_array_create(size_t capacity);
void string_slice_array_free(StringSliceArray *arr);
void string_slice_array_push(StringSliceArray *arr, StringSlice value);

/* ============================================================================ */
/* Account                                                                      */
/* ============================================================================ */

typedef enum {
    ACCOUNT_TYPE_INVALID,
    ASSETS,
    LIABILITIES,
    EQUITY,
    INCOME,
    EXPENSES,
    ACCOUNT_TYPE_COUNT,
} AccountType;

char *account_type_to_string(AccountType type);

typedef struct {
    AccountType type;
    // Full account name, including type prefix (ex: "Assets:")
    StringSlice name;
} Account;

StringSlice account_extract_root(StringSlice name);
AccountType account_extract_type(StringSlice name);

typedef struct {
    Account *data;
    size_t size;
    size_t capacity;
} AccountArray;

AccountArray *account_array_create(size_t capacity);
void account_array_free(AccountArray *arr);
void account_array_push(AccountArray *arr, Account value);

/* ============================================================================ */
/* Transaction                                                                  */
/* ============================================================================ */

typedef struct {
    int year;
    int month;
    int day;
} Date;

typedef struct {
    // Whole number representation of amount (ex: cents for USD)
    long number;
    // Index into an array of currencies
    size_t currency_index;
} Amount;

void cents_to_cstr(long cents, char *buffer, size_t buffer_size);

typedef struct {
    // Index into an array of accounts
    size_t account_index;
    // Unit amount of the position
    Amount amount;
} Posting;

typedef struct {
    Posting *data;
    size_t size;
    size_t capacity;
} PostingArray;

PostingArray *posting_array_create(size_t capacity);
void posting_array_free(PostingArray *arr);
void posting_array_push(PostingArray *arr, Posting value);

typedef enum {
    FLAG_NONE,    // "txn"
    FLAG_OKAY,    // "*"
    FLAG_WARNING, // "!"
} Flag;

typedef struct {
    Date date;
    Flag flag;
    // Optional string representing the payee
    // (if absent set to start NULL and length 0)
    StringSlice payee;
    // String that provides the description of the transaction
    // (can be empty, in which case length is 0 but start not NULL)
    StringSlice narration;
    // Index into an array of postings
    // Note that postings are unique to a transaction, but we use this
    // notably to simplify memory management and avoid small allocations
    size_t postings_start_index;
    size_t postings_count;
    // Line number where transaction was defined
    size_t line;
} Transaction;

typedef struct {
    Transaction *data;
    size_t size;
    size_t capacity;
} TransactionArray;

TransactionArray *transaction_array_create(size_t capacity);
void transaction_array_free(TransactionArray *arr);
void transaction_array_push(TransactionArray *arr, Transaction value);

/* ============================================================================ */
/* Ledger                                                                       */
/* ============================================================================ */

typedef struct {
    // Can be NULL for ledgers created manually (ex: for tests)
    char *file_buffer;
    StringSliceArray *currencies;
    AccountArray *accounts;
    PostingArray *postings;
    TransactionArray *transactions;
} Ledger;

Ledger *ledger_create();
void ledger_free(Ledger *ledger);

/* ============================================================================ */
/* Printer                                                                      */
/* ============================================================================ */

void print_ledger(Ledger *l);

/* ============================================================================ */
/* I/O                                                                          */
/* ============================================================================ */

char *read_file(char *file_path);
Ledger *ledger_create_from_file(char *path);

/* ============================================================================ */
/* Validation                                                                   */
/* ============================================================================ */

bool check_ledger(Ledger *ledger);

/* ============================================================================ */
/* Test                                                                         */
/* ============================================================================ */

int test_ledger();
int test_file(char *path);

#endif
