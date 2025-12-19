#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CURRENCY_ARRAY_INITIAL_CAPACITY 5
#define ACCOUNT_ARRAY_INITIAL_CAPACITY 20
#define POSTING_ARRAY_INITIAL_CAPACITY 100
#define TRANSACTION_ARRAY_INITIAL_CAPACITY 50

#define MAX_TOKEN_LENGTH 256

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
    StringSlice *data;
    size_t size;
    size_t capacity;
} StringSliceArray;

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

    size_t new_capacity = arr->capacity * 2;
    StringSlice *new_data = realloc(arr->data, new_capacity * sizeof(StringSlice));
    if (new_data == NULL) {
        return;
    }

    arr->data = new_data;
    arr->data[arr->size] = value;
    arr->size++;
    arr->capacity = new_capacity;
}

typedef struct {
    Account *data;
    size_t size;
    size_t capacity;
} AccountArray;

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

    size_t new_capacity = arr->capacity * 2;
    Account *new_data = realloc(arr->data, new_capacity * sizeof(Account));
    if (new_data == NULL) {
        return;
    }

    arr->data = new_data;
    arr->data[arr->size] = value;
    arr->size++;
    arr->capacity = new_capacity;
}

typedef struct {
    Posting *data;
    size_t size;
    size_t capacity;
} PostingArray;

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

    size_t new_capacity = arr->capacity * 2;
    Posting *new_data = realloc(arr->data, new_capacity * sizeof(Posting));
    if (new_data == NULL) {
        return;
    }

    arr->data = new_data;
    arr->data[arr->size] = value;
    arr->size++;
    arr->capacity = new_capacity;
}

typedef struct {
    Transaction *data;
    size_t size;
    size_t capacity;
} TransactionArray;

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
    StringSliceArray *currencies;
    AccountArray *accounts;
    PostingArray *postings;
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

    ledger->currencies = string_slice_array_create(CURRENCY_ARRAY_INITIAL_CAPACITY);
    if (ledger->currencies == NULL) {
        free(ledger->file_buffer);
        free(ledger);
        return NULL;
    }

    ledger->accounts = account_array_create(ACCOUNT_ARRAY_INITIAL_CAPACITY);
    if (ledger->accounts == NULL) {
        free(ledger->file_buffer);
        free(ledger->currencies);
        free(ledger);
        return NULL;
    }

    ledger->postings = posting_array_create(POSTING_ARRAY_INITIAL_CAPACITY);
    if (ledger->postings == NULL) {
        free(ledger->file_buffer);
        free(ledger->currencies);
        free(ledger->accounts);
        free(ledger);
        return NULL;
    }

    ledger->transactions = transaction_array_create(TRANSACTION_ARRAY_INITIAL_CAPACITY);
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
        printf("Error allocating ledger\n");
        return EXIT_FAILURE;
    }

    StringSlice currency;
    Account account;
    Posting posting;
    Transaction txn;

    char *cursor = l->file_buffer;

    cursor += sprintf(cursor, "2014-01-01 commodity ");

    char *start = cursor;
    cursor += sprintf(cursor, "USD");
    currency = (StringSlice){
        .start = start,
        .len = (size_t)(cursor - start),
    };
    size_t currency_usd_index = l->currencies->size;
    string_slice_array_push(l->currencies, currency);

    cursor += sprintf(cursor, "\n\n2014-01-01 open ");

    start = cursor;
    cursor += sprintf(cursor, "Assets:Checking");
    account = (Account){
        .type = ASSETS,
        .name =
            (StringSlice){
                .start = start,
                .len = (size_t)(cursor - start),
            },
    };
    size_t account_assets_checking_index = l->accounts->size;
    account_array_push(l->accounts, account);

    cursor += sprintf(cursor, "\n2014-01-01 open ");

    start = cursor;
    cursor += sprintf(cursor, "Expenses:Food");
    account = (Account){
        .type = EXPENSES,
        .name =
            (StringSlice){
                .start = start,
                .len = (size_t)(cursor - start),
            },
    };
    size_t account_expenses_food_index = l->accounts->size;
    account_array_push(l->accounts, account);

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
    txn.postings_start_index = l->postings->size;

    cursor += sprintf(cursor, "  Expenses:Food            -45.12 USD\n");
    posting.account_index = account_expenses_food_index;
    posting.amount = (Amount){
        .number = -4512,
        .currency_index = currency_usd_index,
    };
    posting_array_push(l->postings, posting);
    txn.postings_count++;

    (void)sprintf(cursor, "  Assets:Checking           45.12 USD\n");
    posting.account_index = account_assets_checking_index;
    posting.amount = (Amount){
        .number = 4512,
        .currency_index = currency_usd_index,
    };
    posting_array_push(l->postings, posting);
    txn.postings_count++;

    transaction_array_push(l->transactions, txn);

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
            posting = l->postings->data[t.postings_start_index + j];
            printf(" ");
            print_slice(l->accounts->data[posting.account_index].name);
            printf(" ");
            printf("%ld", posting.amount.number);
            printf(" cents ");
            print_slice(l->currencies->data[posting.amount.currency_index]);
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

int test_file(char *ledger_path) {
    char *file_buffer = read_file(ledger_path);
    if (file_buffer == NULL) {
        printf("Error reading file\n");
        return EXIT_FAILURE;
    }

    printf("%s\n", file_buffer);
    return EXIT_SUCCESS;
}

typedef enum {
    TOKEN_INVALID,
    TOKEN_DATE,     // 2014-01-01
    TOKEN_KEYWORD,  // commodity, open, txn
    TOKEN_ACCOUNT,  // Assets:Checking
    TOKEN_NUMBER,   // -45.12
    TOKEN_CURRENCY, // USD
    TOKEN_STRING,   // "Groceries"
    TOKEN_ASTERISK, // *
    TOKEN_NEWLINE,  // \n
    TOKEN_INDENT,   // leading whitespace
    TOKEN_EOF,      // end of file
} TokenType;

typedef struct {
    TokenType type;
    StringSlice text;
    size_t line;
} Token;

void print_token(Token token) {
    switch (token.type) {
    case TOKEN_INVALID:
        printf("TOKEN_INVALID");
        break;
    case TOKEN_DATE:
        printf("TOKEN_DATE");
        break;
    case TOKEN_KEYWORD:
        printf("TOKEN_KEYWORD");
        break;
    case TOKEN_ACCOUNT:
        printf("TOKEN_ACCOUNT");
        break;
    case TOKEN_NUMBER:
        printf("TOKEN_NUMBER");
        break;
    case TOKEN_CURRENCY:
        printf("TOKEN_CURRENCY");
        break;
    case TOKEN_STRING:
        printf("TOKEN_STRING");
        break;
    case TOKEN_ASTERISK:
        printf("TOKEN_ASTERISK");
        break;
    case TOKEN_NEWLINE:
        printf("TOKEN_NEWLINE");
        break;
    case TOKEN_INDENT:
        printf("TOKEN_INDENT");
        break;
    case TOKEN_EOF:
        printf("TOKEN_EOF");
        break;
    }

    if (token.type == TOKEN_EOF) {
        printf("\n");
        return;
    }

    printf(" [line %zu]", token.line);

    if (token.type == TOKEN_NEWLINE) {
        printf("\n");
        return;
    }

    if (token.type == TOKEN_INVALID) {
        printf(" '%c'\n", *token.text.start);
        return;
    }

    if (token.type == TOKEN_INDENT) {
        printf(" (size %zu)\n", token.text.len);
        return;
    }

    char token_text[MAX_TOKEN_LENGTH] = {0};
    (void)snprintf(token_text, MAX_TOKEN_LENGTH, "%.*s", (int)token.text.len, token.text.start);
    printf(" %s\n", token_text);
}

typedef struct {
    const char *buffer;
    const char *current;
    size_t line;
} Scanner;

Scanner scanner_init(char *buffer) {
    return (Scanner){
        .buffer = buffer,
        .current = buffer,
        .line = 1,
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

Token scanner_next_token(Scanner *s) {
    scanner_skip_whitespace(s);

    char c = *s->current;
    Token t = {0};

    if (c == '\0') {
        t.type = TOKEN_EOF;
        return t;
    }

    if (c == '\n') {
        t.type = TOKEN_NEWLINE;
        t.text = (StringSlice){
            .start = s->current,
            .len = 1,
        };
        t.line = s->line;
        s->current++;
        s->line++;
        return t;
    }

    if (c == '*') {
        t.type = TOKEN_ASTERISK;
        t.text = (StringSlice){
            .start = s->current,
            .len = 1,
        };
        t.line = s->line;
        s->current++;
        return t;
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

    return EXIT_SUCCESS;
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
        // return test_data();
        char *ledger_path = get_ledger_path(argc, argv);
        if (ledger_path == NULL) {
            return EXIT_FAILURE;
        }
        // return test_file(ledger_path);
        return test_scanner(ledger_path);
    }

    printf("Unknown command: %s\n", command);
    printf("\n");
    print_usage();
    return EXIT_FAILURE;
}
