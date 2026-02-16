#include "ledger.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CURRENCY_ARRAY_INITIAL_CAPACITY 5
#define ACCOUNT_ARRAY_INITIAL_CAPACITY 20
#define POSTING_ARRAY_INITIAL_CAPACITY 100
#define TRANSACTION_ARRAY_INITIAL_CAPACITY 50

#define AMOUNT_DOLLAR_CENTS_RATIO 100

/* ============================================================================ */
/* String                                                                       */
/* ============================================================================ */

void slice_to_cstr(StringSlice slice, char *buffer, size_t buffer_size) {
    assert(buffer_size > slice.len && "Buffer size needs to be greater than slice length");
    (void)snprintf(buffer, buffer_size, "%.*s", (int)slice.len, slice.start);
}

bool slice_equals_cstr(StringSlice slice, char *str) {
    size_t str_len = strlen(str);
    if (slice.len != str_len) {
        return false;
    }

    if (memcmp(slice.start, str, slice.len) != 0) {
        return false;
    }

    return true;
}

bool slice_equals(StringSlice s1, StringSlice s2) {
    if (s1.len != s2.len) {
        return false;
    }

    if (memcmp(s1.start, s2.start, s1.len) != 0) {
        return false;
    }

    return true;
}

StringSliceArray *string_slice_array_create(size_t capacity) {
    StringSliceArray *arr = malloc(sizeof(StringSliceArray));
    if (arr == NULL) {
        return NULL;
    }

    arr->data = malloc(capacity * sizeof(StringSlice));
    if (arr->data == NULL) {
        free(arr);
        return NULL;
    }

    arr->size = 0;
    arr->capacity = capacity;
    return arr;
}

void string_slice_array_free(StringSliceArray *arr) {
    if (arr == NULL) {
        return;
    }

    free(arr->data);
    free(arr);
}

void string_slice_array_push(StringSliceArray *arr, StringSlice value) {
    if (arr->size < arr->capacity) {
        arr->data[arr->size] = value;
        arr->size++;
        return;
    }

    size_t new_capacity = arr->capacity == 0 ? 1 : arr->capacity * 2;
    StringSlice *new_data = realloc(arr->data, new_capacity * sizeof(StringSlice));
    if (new_data == NULL) {
        return;
    }

    arr->data = new_data;
    // NOLINTNEXTLINE(clang-analyzer-security.ArrayBound)
    arr->data[arr->size] = value;
    arr->size++;
    arr->capacity = new_capacity;
}

/* ============================================================================ */
/* Account                                                                      */
/* ============================================================================ */

char *account_type_to_string(AccountType type) {
    switch (type) {
    case ASSETS:
        return "Assets";
    case LIABILITIES:
        return "Liabilities";
    case EQUITY:
        return "Equity";
    case INCOME:
        return "Income";
    case EXPENSES:
        return "Expenses";
    default:
        return "UNKNOWN";
    }
}

StringSlice account_extract_root(StringSlice name) {
    size_t i = 0;
    while (i < name.len) {
        if (name.start[i] == ':') {
            break;
        }
        i++;
    }

    return (StringSlice){
        .start = name.start,
        .len = i,
    };
}

AccountType account_extract_type(StringSlice name) {
    StringSlice root = account_extract_root(name);
    if (slice_equals_cstr(root, "Assets")) {
        return ASSETS;
    }
    if (slice_equals_cstr(root, "Liabilities")) {
        return LIABILITIES;
    }
    if (slice_equals_cstr(root, "Equity")) {
        return EQUITY;
    }
    if (slice_equals_cstr(root, "Income")) {
        return INCOME;
    }
    if (slice_equals_cstr(root, "Expenses")) {
        return EXPENSES;
    }
    return ACCOUNT_TYPE_INVALID;
}

AccountArray *account_array_create(size_t capacity) {
    AccountArray *arr = malloc(sizeof(AccountArray));
    if (arr == NULL) {
        return NULL;
    }

    arr->data = malloc(capacity * sizeof(Account));
    if (arr->data == NULL) {
        free(arr);
        return NULL;
    }

    arr->size = 0;
    arr->capacity = capacity;
    return arr;
}

void account_array_free(AccountArray *arr) {
    if (arr == NULL) {
        return;
    }

    free(arr->data);
    free(arr);
}

void account_array_push(AccountArray *arr, Account value) {
    if (arr->size < arr->capacity) {
        arr->data[arr->size] = value;
        arr->size++;
        return;
    }

    size_t new_capacity = arr->capacity == 0 ? 1 : arr->capacity * 2;
    Account *new_data = realloc(arr->data, new_capacity * sizeof(Account));
    if (new_data == NULL) {
        return;
    }

    arr->data = new_data;
    // NOLINTNEXTLINE(clang-analyzer-security.ArrayBound)
    arr->data[arr->size] = value;
    arr->size++;
    arr->capacity = new_capacity;
}

/* ============================================================================ */
/* Transaction                                                                  */
/* ============================================================================ */

void cents_to_cstr(long cents, char *buffer, size_t buffer_size) {
    long dollars = cents / AMOUNT_DOLLAR_CENTS_RATIO;
    long remainder = llabs(cents % AMOUNT_DOLLAR_CENTS_RATIO);
    (void)snprintf(buffer, buffer_size, "%ld.%02ld", dollars, remainder);
}

PostingArray *posting_array_create(size_t capacity) {
    PostingArray *arr = malloc(sizeof(PostingArray));
    if (arr == NULL) {
        return NULL;
    }

    arr->data = malloc(capacity * sizeof(Posting));
    if (arr->data == NULL) {
        free(arr);
        return NULL;
    }

    arr->size = 0;
    arr->capacity = capacity;
    return arr;
}

void posting_array_free(PostingArray *arr) {
    if (arr == NULL) {
        return;
    }

    free(arr->data);
    free(arr);
}

void posting_array_push(PostingArray *arr, Posting value) {
    if (arr->size < arr->capacity) {
        arr->data[arr->size] = value;
        arr->size++;
        return;
    }

    size_t new_capacity = arr->capacity == 0 ? 1 : arr->capacity * 2;
    Posting *new_data = realloc(arr->data, new_capacity * sizeof(Posting));
    if (new_data == NULL) {
        return;
    }

    arr->data = new_data;
    // NOLINTNEXTLINE(clang-analyzer-security.ArrayBound)
    arr->data[arr->size] = value;
    arr->size++;
    arr->capacity = new_capacity;
}

char *flag_to_string(Flag flag) {
    switch (flag) {
    case FLAG_NONE:
        return "txn";
    case FLAG_OKAY:
        return "*";
    case FLAG_WARNING:
        return "!";
    default:
        return "UNKNOWN";
    }
}

TransactionArray *transaction_array_create(size_t capacity) {
    TransactionArray *arr = malloc(sizeof(TransactionArray));
    if (arr == NULL) {
        return NULL;
    }

    arr->data = malloc(capacity * sizeof(Transaction));
    if (arr->data == NULL) {
        free(arr);
        return NULL;
    }

    arr->size = 0;
    arr->capacity = capacity;
    return arr;
}

void transaction_array_free(TransactionArray *arr) {
    if (arr == NULL) {
        return;
    }

    free(arr->data);
    free(arr);
}

void transaction_array_push(TransactionArray *arr, Transaction value) {
    if (arr->size < arr->capacity) {
        arr->data[arr->size] = value;
        arr->size++;
        return;
    }

    size_t new_capacity = arr->capacity == 0 ? 1 : arr->capacity * 2;
    Transaction *new_data = realloc(arr->data, new_capacity * sizeof(Transaction));
    if (new_data == NULL) {
        return;
    }

    arr->data = new_data;
    // NOLINTNEXTLINE(clang-analyzer-security.ArrayBound)
    arr->data[arr->size] = value;
    arr->size++;
    arr->capacity = new_capacity;
}

/* ============================================================================ */
/* Ledger                                                                       */
/* ============================================================================ */

void ledger_free(Ledger *ledger) {
    if (ledger == NULL) {
        return;
    }

    string_slice_array_free(ledger->currencies);
    account_array_free(ledger->accounts);
    posting_array_free(ledger->postings);
    transaction_array_free(ledger->transactions);

    if (ledger->file_buffer != NULL) {
        free(ledger->file_buffer);
    }

    free(ledger);
}

Ledger *ledger_create(void) {
    Ledger *ledger = malloc(sizeof(Ledger));
    if (ledger == NULL) {
        return NULL;
    }

    ledger->file_buffer = NULL;
    ledger->currencies = NULL;
    ledger->accounts = NULL;
    ledger->postings = NULL;
    ledger->transactions = NULL;

    ledger->currencies = string_slice_array_create(CURRENCY_ARRAY_INITIAL_CAPACITY);
    if (ledger->currencies == NULL) {
        ledger_free(ledger);
        return NULL;
    }

    ledger->accounts = account_array_create(ACCOUNT_ARRAY_INITIAL_CAPACITY);
    if (ledger->accounts == NULL) {
        ledger_free(ledger);
        return NULL;
    }

    ledger->postings = posting_array_create(POSTING_ARRAY_INITIAL_CAPACITY);
    if (ledger->postings == NULL) {
        ledger_free(ledger);
        return NULL;
    }

    ledger->transactions = transaction_array_create(TRANSACTION_ARRAY_INITIAL_CAPACITY);
    if (ledger->transactions == NULL) {
        ledger_free(ledger);
        return NULL;
    }

    return ledger;
}

/* ============================================================================ */
/* Printer                                                                      */
/* ============================================================================ */

void print_slice(StringSlice slice) {
    printf("%.*s", (int)slice.len, slice.start);
}

void print_amount(Amount amount, StringSliceArray *currencies) {
    long cents = amount.number;
    long dollars = cents / AMOUNT_DOLLAR_CENTS_RATIO;
    long remainder = llabs(cents % AMOUNT_DOLLAR_CENTS_RATIO);
    printf("%ld.%02ld", dollars, remainder);
    printf(" ");
    print_slice(currencies->data[amount.currency_index]);
}

void print_transaction(Ledger *l, Transaction *t) {
    printf("Date: %d-%02d-%02d\n", t->date.year, t->date.month, t->date.day);
    printf("Flag: %s\n", flag_to_string(t->flag));
    printf("Payee: ");
    if (t->payee.start == NULL) {
        printf("NULL\n");
    } else {
        print_slice(t->payee);
        printf("\n");
    }
    printf("Narration: ");
    print_slice(t->narration);
    printf("\n");
    printf("Postings:\n");
    for (size_t j = 0; j < t->postings_count; j++) {
        Posting *p = &l->postings->data[t->postings_start_index + j];
        printf(" ");
        print_slice(l->accounts->data[p->account_index].name);
        printf(" ");
        print_amount(p->amount, l->currencies);
        printf("\n");
    }
}

void print_ledger(Ledger *l) {
    printf("=================================\n");
    printf("Currencies\n");
    printf("=================================\n");
    for (size_t i = 0; i < l->currencies->size; i++) {
        print_slice(l->currencies->data[i]);
        printf("\n");
    }
    printf("\n");

    printf("=================================\n");
    printf("Accounts\n");
    printf("=================================\n");
    for (size_t i = 0; i < l->accounts->size; i++) {
        print_slice(l->accounts->data[i].name);
        printf("\n");
    }
    printf("\n");

    printf("=================================\n");
    printf("Transactions\n");
    printf("=================================\n");
    for (size_t i = 0; i < l->transactions->size; i++) {
        Transaction *t = &l->transactions->data[i];
        print_transaction(l, t);
        printf("\n");
    }
}

/* ============================================================================ */
/* I/O                                                                          */
/* ============================================================================ */

char *read_file(char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", file_path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        printf("Error seeking to end of file: %s\n", file_path);
        (void)fclose(file);
        return NULL;
    }

    long ftell_size = ftell(file);
    if (ftell_size == -1L) {
        printf("Error reading size of file: %s\n", file_path);
        (void)fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        printf("Error seeking to beginning of file: %s\n", file_path);
        (void)fclose(file);
        return NULL;
    }

    size_t file_size = (size_t)ftell_size;
    char *buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        (void)fclose(file);
        printf("Error allocating buffer for file: %s\n", file_path);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        if (feof(file)) {
            printf("Reached end-of-file before reading %zu bytes from file: %s\n",
                   file_size,
                   file_path);
        } else if (ferror(file)) {
            printf("Error reading file: %s\n", file_path);
        } else {
            printf("Expected to read %zu bytes got %zu from file: %s\n",
                   file_size,
                   bytes_read,
                   file_path);
        }

        free(buffer);
        (void)fclose(file);
        return NULL;
    }

    (void)fclose(file);

    // buffer capacity = file_size + 1, so this is safe
    // NOLINTNEXTLINE(clang-analyzer-security.ArrayBound)
    buffer[file_size] = '\0';

    return buffer;
}

Ledger *ledger_create_from_file(char *path) {
    char *file_buffer = read_file(path);
    if (file_buffer == NULL) {
        printf("Error reading file\n");
        return NULL;
    }

    Ledger *ledger = ledger_create();
    if (ledger == NULL) {
        printf("Error allocating ledger\n");
        free(file_buffer);
        return NULL;
    }

    ledger->file_buffer = file_buffer;
    return ledger;
}

/* ============================================================================ */
/* Validation                                                                   */
/* ============================================================================ */

bool check_transactions_balanced(Ledger *l) {
    bool pass = true;
    for (size_t i = 0; i < l->transactions->size; i++) {
        Transaction *t = &l->transactions->data[i];
        long sum = 0;
        for (size_t j = 0; j < t->postings_count; j++) {
            Posting *p = &l->postings->data[t->postings_start_index + j];
            sum += p->amount.number;
        }
        if (sum != 0) {
            printf("Line %zu: Transaction does not balance (%ld cents):\n\n", t->line, sum);
            print_transaction(l, t);
            printf("\n");

            pass = false;
        }
    }
    return pass;
}

bool check_single_currency(Ledger *l) {
    size_t currency_count = l->currencies->size;
    if (currency_count > 1) {
        printf("Only a single currency per ledger is supported, got %zu\n\n", currency_count);
        return false;
    }
    return true;
}

bool check_ledger(Ledger *ledger) {
    if (!check_single_currency(ledger)) {
        return false;
    }

    if (!check_transactions_balanced(ledger)) {
        return false;
    }

    return true;
}

/* ============================================================================ */
/* Test                                                                         */
/* ============================================================================ */

int test_ledger(void) {
    Ledger *l = ledger_create();
    if (l == NULL) {
        printf("Error allocating ledger\n");
        return EXIT_FAILURE;
    }

    StringSlice currency;
    Account account;
    Posting posting;
    Transaction txn;

    currency = (StringSlice){
        .start = "USD",
        .len = strlen("USD"),
    };
    size_t currency_usd_index = l->currencies->size;
    string_slice_array_push(l->currencies, currency);

    account = (Account){
        .type = ASSETS,
        .name =
            (StringSlice){
                .start = "Assets:Checking",
                .len = strlen("Assets:Checking"),
            },
    };
    size_t account_assets_checking_index = l->accounts->size;
    account_array_push(l->accounts, account);

    account = (Account){
        .type = EXPENSES,
        .name =
            (StringSlice){
                .start = "Expenses:Food",
                .len = strlen("Expenses:Food"),
            },
    };
    size_t account_expenses_food_index = l->accounts->size;
    account_array_push(l->accounts, account);

    txn.date = (Date){
        .year = 2014,
        .month = 1,
        .day = 5,
    };
    txn.flag = FLAG_OKAY;
    txn.payee = (StringSlice){
        .start = "Whole Foods",
        .len = strlen("Whole Foods"),
    };
    txn.narration = (StringSlice){
        .start = "Groceries",
        .len = strlen("Groceries"),
    };

    txn.postings_count = 0;
    txn.postings_start_index = l->postings->size;

    posting.account_index = account_expenses_food_index;
    posting.amount = (Amount){
        .number = -4512,
        .currency_index = currency_usd_index,
    };
    posting_array_push(l->postings, posting);
    txn.postings_count++;

    posting.account_index = account_assets_checking_index;
    posting.amount = (Amount){
        .number = 4512,
        .currency_index = currency_usd_index,
    };
    posting_array_push(l->postings, posting);
    txn.postings_count++;

    txn.line = 0;

    transaction_array_push(l->transactions, txn);

    print_ledger(l);

    ledger_free(l);

    return EXIT_SUCCESS;
}

int test_file(char *path) {
    char *file_buffer = read_file(path);
    if (file_buffer == NULL) {
        printf("Error reading file\n");
        return EXIT_FAILURE;
    }

    printf("%s\n", file_buffer);
    free(file_buffer);
    return EXIT_SUCCESS;
}
