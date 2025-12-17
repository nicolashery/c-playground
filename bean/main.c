#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    // Pointer into file buffer
    // Ownership: The string slice does not own this buffer and memory is not
    // freed when the slice is freed
    const char *start;
    // Length in bytes
    size_t len;
} StringSlice;

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
    // (can be empty, in which cast length is 0 but start not NULL)
    StringSlice narration;
    // Array of postings for this transaction
    // Ownership: The transaction owns these postings, memory is allocated and
    // freed at the same time as the transaction
    Posting *postings;
    size_t postings_count;
} Transaction;

void print_slice(StringSlice slice) {
    printf("%.*s", (int)slice.len, slice.start);
}

int test_data() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // Simulate file buffer dynamically read in memory
    char *file_buffer = malloc(1024);
    if (file_buffer == NULL) {
        printf("ERROR: Could not allocate file buffer\n");
        return EXIT_FAILURE;
    }

    // Simulate dynamic array for currencies
    // (will not grow in this test)
    StringSlice *currencies = malloc(10 * sizeof(StringSlice));
    if (currencies == NULL) {
        printf("ERROR: Could not allocate currencies array\n");
        free(file_buffer);
        return EXIT_FAILURE;
    }

    // Simulate dynamic array for accounts
    // (will not grow in this test)
    Account *accounts = malloc(10 * sizeof(Account));
    if (accounts == NULL) {
        printf("ERROR: Could not allocate accounts array\n");
        free(currencies);
        free(file_buffer);
        return EXIT_FAILURE;
    }

    // Simulate dynamic array for transactions
    // (will not grow in this test)
    Transaction *transactions = malloc(10 * sizeof(Transaction));
    if (accounts == NULL) {
        printf("ERROR: Could not allocate transactions array\n");
        free(currencies);
        free(accounts);
        free(file_buffer);
        return EXIT_FAILURE;
    }
    size_t transactions_count = 0;

    char *cursor = file_buffer;

    cursor += sprintf(cursor, "2014-01-01 commodity ");

    char *start = cursor;
    cursor += sprintf(cursor, "USD");
    currencies[0] = (StringSlice){
        .start = start,
        .len = (size_t)(cursor - start),
    };

    cursor += sprintf(cursor, "\n\n2014-01-01 open ");

    start = cursor;
    cursor += sprintf(cursor, "Assets:Checking");
    accounts[0] = (Account){
        .type = ASSETS,
        .name =
            (StringSlice){
                .start = start,
                .len = (size_t)(cursor - start),
            },
    };

    cursor += sprintf(cursor, "\n2014-01-01 open ");

    start = cursor;
    cursor += sprintf(cursor, "Expenses:Food");
    accounts[1] = (Account){
        .type = EXPENSES,
        .name =
            (StringSlice){
                .start = start,
                .len = (size_t)(cursor - start),
            },
    };

    cursor += sprintf(cursor, "\n\n2014-01-05 * \"");

    start = cursor;
    cursor += sprintf(cursor, "Whole Foods");
    StringSlice payee = {
        .start = start,
        .len = (size_t)(cursor - start),
    };

    cursor += sprintf(cursor, "\" \"");

    start = cursor;
    cursor += sprintf(cursor, "Groceries");
    StringSlice narration = {
        .start = start,
        .len = (size_t)(cursor - start),
    };

    cursor += sprintf(cursor, "\"\n");
    cursor += sprintf(cursor, "  Expenses:Food            -45.12 USD\n");
    (void)sprintf(cursor, "  Assets:Checking           45.12 USD\n");
    size_t postings_count = 2;
    Posting *postings = malloc(postings_count * sizeof(Posting));
    if (postings == NULL) {
        printf("ERROR: Could not allocate postings array\n");
        free(currencies);
        free(accounts);
        for (size_t i = 0; i < transactions_count; i++) {
            free(transactions[i].postings);
        }
        free(transactions);
        free(file_buffer);
        return EXIT_FAILURE;
    }
    postings[0] = (Posting){
        .account_index = 1,
        .amount =
            (Amount){
                .number = -4512,
                .currency_index = 0,
            },
    };
    postings[1] = (Posting){
        .account_index = 0,
        .amount =
            (Amount){
                .number = 4512,
                .currency_index = 0,
            },
    };
    transactions[transactions_count++] = (Transaction){
        .date =
            (Date){
                .year = 2014,
                .month = 1,
                .day = 5,
            },
        .flag = FLAG_OKAY,
        .payee = payee,
        .narration = narration,
        .postings = postings,
        .postings_count = postings_count,
    };

    printf("=================================\n");
    printf("Currencies\n");
    printf("=================================\n");
    print_slice(currencies[0]);
    printf("\n");
    printf("\n");

    printf("=================================\n");
    printf("Accounts\n");
    printf("=================================\n");
    print_slice(accounts[0].name);
    printf("\n");
    print_slice(accounts[1].name);
    printf("\n");
    printf("\n");

    printf("=================================\n");
    printf("Transactions\n");
    printf("=================================\n");
    Transaction t = transactions[0];
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
    for (size_t i = 0; i < t.postings_count; i++) {
        printf(" ");
        print_slice(accounts[t.postings[i].account_index].name);
        printf(" ");
        printf("%ld", t.postings[i].amount.number);
        printf(" cents ");
        print_slice(currencies[t.postings[i].amount.currency_index]);
        printf("\n");
    }
    printf("\n");

    printf("=================================\n");
    printf("File buffer\n");
    printf("=================================\n");
    printf("%s\n", file_buffer);

    free(currencies);
    free(accounts);
    for (size_t i = 0; i < transactions_count; i++) {
        free(transactions[i].postings);
    }
    free(transactions);
    free(file_buffer);
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
