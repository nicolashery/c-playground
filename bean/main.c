#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CURRENCY_ARRAY_INITIAL_CAPACITY 5
#define ACCOUNT_ARRAY_INITIAL_CAPACITY 20
#define POSTING_ARRAY_INITIAL_CAPACITY 100
#define TRANSACTION_ARRAY_INITIAL_CAPACITY 50

typedef struct {
    // Pointer into file buffer
    const char *start;
    // Length in bytes
    size_t len;
} StringSlice;

void print_slice(StringSlice slice) {
    printf("%.*s", (int)slice.len, slice.start);
}

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

typedef enum {
    ASSETS,
    LIABILITIES,
    EQUITY,
    INCOME,
    EXPENSES,
} AccountType;

typedef struct {
    AccountType type;
    // Full account name, including type prefix (ex: "Assets:")
    StringSlice name;
} Account;

typedef struct {
    // Index into an array of accounts
    size_t account_index;
    // Unit amount of the position
    Amount amount;
} Posting;

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
} Transaction;

typedef struct {
    Transaction *data;
    size_t size;
    size_t capacity;
} TransactionArray;

TransactionArray *transaction_array_create() {
    TransactionArray *arr = malloc(sizeof(TransactionArray));
    if (arr == NULL) {
        return NULL;
    }

    arr->data = malloc(TRANSACTION_ARRAY_INITIAL_CAPACITY * sizeof(Transaction));
    if (arr->data == NULL) {
        free(arr);
        return NULL;
    }

    arr->size = 0;
    arr->capacity = TRANSACTION_ARRAY_INITIAL_CAPACITY;
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

    size_t new_capacity = arr->capacity * 2;
    Transaction *new_data = realloc(arr->data, new_capacity * sizeof(Transaction));
    if (new_data == NULL) {
        return;
    }

    arr->data = new_data;
    arr->data[arr->size] = value;
    arr->size++;
    arr->capacity = new_capacity;
}

typedef struct {
    char *file_buffer;
    StringSlice *currencies;
    Account *accounts;
    Posting *postings;
    TransactionArray *transactions;
} Ledger;

Ledger *ledger_create() {
    Ledger *ledger = malloc(sizeof(Ledger));
    if (ledger == NULL) {
        return NULL;
    }

    ledger->file_buffer = malloc(1024);
    if (ledger->file_buffer == NULL) {
        free(ledger);
        return NULL;
    }

    ledger->currencies = malloc(CURRENCY_ARRAY_INITIAL_CAPACITY * sizeof(StringSlice));
    if (ledger->currencies == NULL) {
        free(ledger->file_buffer);
        free(ledger);
        return NULL;
    }

    ledger->accounts = malloc(ACCOUNT_ARRAY_INITIAL_CAPACITY * sizeof(Account));
    if (ledger->accounts == NULL) {
        free(ledger->file_buffer);
        free(ledger->currencies);
        free(ledger);
        return NULL;
    }

    ledger->postings = malloc(POSTING_ARRAY_INITIAL_CAPACITY * sizeof(Posting));
    if (ledger->postings == NULL) {
        free(ledger->file_buffer);
        free(ledger->currencies);
        free(ledger->accounts);
        free(ledger);
        return NULL;
    }

    ledger->transactions = transaction_array_create();
    if (ledger->transactions == NULL) {
        free(ledger->file_buffer);
        free(ledger->currencies);
        free(ledger->accounts);
        free(ledger->postings);
        free(ledger);
        return NULL;
    }

    return ledger;
}

void ledger_free(Ledger *ledger) {
    if (ledger == NULL) {
        return;
    }

    free(ledger->file_buffer);
    free(ledger->currencies);
    free(ledger->accounts);
    free(ledger->postings);
    transaction_array_free(ledger->transactions);
    free(ledger);
}

int test_data() {
// Ignore warnings for sprintf
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    Ledger *l = ledger_create();
    if (l == NULL) {
        printf("ERROR: Could not allocate ledger\n");
        return EXIT_FAILURE;
    }

    size_t currencies_count = 0;
    size_t accounts_count = 0;
    size_t postings_count = 0;
    Transaction txn;

    char *cursor = l->file_buffer;

    cursor += sprintf(cursor, "2014-01-01 commodity ");

    char *start = cursor;
    cursor += sprintf(cursor, "USD");
    l->currencies[currencies_count] = (StringSlice){
        .start = start,
        .len = (size_t)(cursor - start),
    };
    size_t currency_usd_index = currencies_count;
    currencies_count++;

    cursor += sprintf(cursor, "\n\n2014-01-01 open ");

    start = cursor;
    cursor += sprintf(cursor, "Assets:Checking");
    l->accounts[accounts_count] = (Account){
        .type = ASSETS,
        .name =
            (StringSlice){
                .start = start,
                .len = (size_t)(cursor - start),
            },
    };
    size_t account_assets_checking_index = accounts_count;
    accounts_count++;

    cursor += sprintf(cursor, "\n2014-01-01 open ");

    start = cursor;
    cursor += sprintf(cursor, "Expenses:Food");
    l->accounts[accounts_count] = (Account){
        .type = EXPENSES,
        .name =
            (StringSlice){
                .start = start,
                .len = (size_t)(cursor - start),
            },
    };
    size_t account_expenses_food_index = accounts_count;
    accounts_count++;

    cursor += sprintf(cursor, "\n\n2014-01-05 * \"");
    txn.date = (Date){
        .year = 2014,
        .month = 1,
        .day = 5,
    };
    txn.flag = FLAG_OKAY;

    start = cursor;
    cursor += sprintf(cursor, "Whole Foods");
    txn.payee = (StringSlice){
        .start = start,
        .len = (size_t)(cursor - start),
    };

    cursor += sprintf(cursor, "\" \"");

    start = cursor;
    cursor += sprintf(cursor, "Groceries");
    txn.narration = (StringSlice){
        .start = start,
        .len = (size_t)(cursor - start),
    };

    cursor += sprintf(cursor, "\"\n");
    txn.postings_count = 0;
    txn.postings_start_index = postings_count;

    cursor += sprintf(cursor, "  Expenses:Food            -45.12 USD\n");
    txn.postings_count++;
    Posting *p = &l->postings[postings_count];
    postings_count++;
    p->account_index = account_expenses_food_index;
    p->amount = (Amount){
        .number = -4512,
        .currency_index = currency_usd_index,
    };

    (void)sprintf(cursor, "  Assets:Checking           45.12 USD\n");
    txn.postings_count++;
    p = &l->postings[postings_count];
    postings_count++;
    p->account_index = account_assets_checking_index;
    p->amount = (Amount){
        .number = 4512,
        .currency_index = currency_usd_index,
    };

    transaction_array_push(l->transactions, txn);

    printf("=================================\n");
    printf("Currencies\n");
    printf("=================================\n");
    for (size_t i = 0; i < currencies_count; i++) {
        print_slice(l->currencies[i]);
        printf("\n");
    }
    printf("\n");

    printf("=================================\n");
    printf("Accounts\n");
    printf("=================================\n");
    for (size_t i = 0; i < accounts_count; i++) {
        print_slice(l->accounts[i].name);
        printf("\n");
    }
    printf("\n");

    printf("=================================\n");
    printf("Transactions\n");
    printf("=================================\n");
    for (size_t i = 0; i < l->transactions->size; i++) {
        Transaction t = l->transactions->data[i];
        printf("Date: %d-%02d-%02d\n", t.date.year, t.date.month, t.date.day);
        printf("Payee: ");
        if (t.payee.start == NULL) {
            printf("NULL\n");
        } else {
            print_slice(t.payee);
            printf("\n");
        }
        printf("Narration: ");
        print_slice(t.narration);
        printf("\n");
        printf("Postings:\n");
        for (size_t j = 0; j < t.postings_count; j++) {
            Posting posting = l->postings[t.postings_start_index + j];
            printf(" ");
            print_slice(l->accounts[posting.account_index].name);
            printf(" ");
            printf("%ld", posting.amount.number);
            printf(" cents ");
            print_slice(l->currencies[posting.amount.currency_index]);
            printf("\n");
        }
    }
    printf("\n");

    printf("=================================\n");
    printf("File buffer\n");
    printf("=================================\n");
    printf("%s\n", l->file_buffer);

    ledger_free(l);
    return EXIT_SUCCESS;
#pragma clang diagnostic pop
}

int run_check(char *ledger_path) {
    (void)ledger_path;
    printf("Not implemented\n");
    return EXIT_FAILURE;
}

int run_balance(char *ledger_path) {
    (void)ledger_path;
    printf("Not implemented\n");
    return EXIT_FAILURE;
}

void print_usage() {
    printf("Usage:\n\n");
    printf("  bean --help|-h              Print this text\n");
    printf("  bean check <ledger_file>    Parse and check ledger file\n");
    printf("  bean balance <ledger_file>  Compute per-account balances\n");
    printf("\n");
}

char *get_ledger_path(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Missing ledger file argument\n");
        printf("\n");
        print_usage();
        return NULL;
    }

    return argv[2];
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Missing command argument\n");
        printf("\n");
        print_usage();
        return EXIT_FAILURE;
    }

    char *command = argv[1];
    if (strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0) {
        print_usage();
        return EXIT_SUCCESS;
    }

    if (strcmp(command, "check") == 0) {
        char *ledger_path = get_ledger_path(argc, argv);
        if (ledger_path == NULL) {
            return EXIT_FAILURE;
        }
        return run_check(ledger_path);
    }

    if (strcmp(command, "balance") == 0) {
        char *ledger_path = get_ledger_path(argc, argv);
        if (ledger_path == NULL) {
            return EXIT_FAILURE;
        }
        return run_balance(ledger_path);
    }

    if (strcmp(command, "test") == 0) {
        return test_data();
    }

    printf("Unknown command: %s\n", command);
    printf("\n");
    print_usage();
    return EXIT_FAILURE;
}
