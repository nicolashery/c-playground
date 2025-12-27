#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CURRENCY_ARRAY_INITIAL_CAPACITY 5
#define ACCOUNT_ARRAY_INITIAL_CAPACITY 20
#define POSTING_ARRAY_INITIAL_CAPACITY 100
#define TRANSACTION_ARRAY_INITIAL_CAPACITY 50
#define TOKEN_ARRAY_INITIAL_CAPACITY 100

#define MAX_TOKEN_TYPE_LENGTH 16
#define MAX_TOKEN_LENGTH 256
#define MAX_ERROR_MESSAGE_LENGTH 256

typedef struct {
    // Pointer into file buffer
    const char *start;
    // Length in bytes
    size_t len;
} StringSlice;

void print_slice(StringSlice slice) {
    printf("%.*s", (int)slice.len, slice.start);
}

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
    ACCOUNT_TYPE_INVALID,
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

typedef struct {
    // Ownership: File buffer is not owned by Ledger and needs to be freed by caller
    char *file_buffer;
    StringSliceArray *currencies;
    AccountArray *accounts;
    PostingArray *postings;
    TransactionArray *transactions;
} Ledger;

Ledger *ledger_create(char *file_buffer) {
    Ledger *ledger = malloc(sizeof(Ledger));
    if (ledger == NULL) {
        return NULL;
    }

    ledger->file_buffer = file_buffer;

    ledger->currencies = string_slice_array_create(CURRENCY_ARRAY_INITIAL_CAPACITY);
    if (ledger->currencies == NULL) {
        free(ledger);
        return NULL;
    }

    ledger->accounts = account_array_create(ACCOUNT_ARRAY_INITIAL_CAPACITY);
    if (ledger->accounts == NULL) {
        string_slice_array_free(ledger->currencies);
        free(ledger);
        return NULL;
    }

    ledger->postings = posting_array_create(POSTING_ARRAY_INITIAL_CAPACITY);
    if (ledger->postings == NULL) {
        string_slice_array_free(ledger->currencies);
        account_array_free(ledger->accounts);
        free(ledger);
        return NULL;
    }

    ledger->transactions = transaction_array_create(TRANSACTION_ARRAY_INITIAL_CAPACITY);
    if (ledger->transactions == NULL) {
        string_slice_array_free(ledger->currencies);
        account_array_free(ledger->accounts);
        posting_array_free(ledger->postings);
        free(ledger);
        return NULL;
    }

    return ledger;
}

void ledger_free(Ledger *ledger) {
    if (ledger == NULL) {
        return;
    }

    string_slice_array_free(ledger->currencies);
    account_array_free(ledger->accounts);
    posting_array_free(ledger->postings);
    transaction_array_free(ledger->transactions);
    free(ledger);
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
        Transaction t = l->transactions->data[i];
        printf("Date: %d-%02d-%02d\n", t.date.year, t.date.month, t.date.day);
        printf("Flag: %s\n", flag_to_string(t.flag));
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
            Posting p = l->postings->data[t.postings_start_index + j];
            printf(" ");
            print_slice(l->accounts->data[p.account_index].name);
            printf(" ");
            printf("%ld", p.amount.number);
            printf(" cents ");
            print_slice(l->currencies->data[p.amount.currency_index]);
            printf("\n");
        }
        printf("\n");
    }
}

void print_file_buffer(char *file_buffer) {
    printf("\n");

    printf("=================================\n");
    printf("File buffer\n");
    printf("=================================\n");
    printf("%s\n", file_buffer);
}

int test_ledger() {
// Ignore warnings for sprintf
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    char *file_buffer = malloc(1024);
    if (file_buffer == NULL) {
        printf("Error allocating file buffer\n");
        return EXIT_FAILURE;
    }

    Ledger *l = ledger_create(file_buffer);
    if (l == NULL) {
        printf("Error allocating ledger\n");
        free(file_buffer);
        return EXIT_FAILURE;
    }

    StringSlice currency;
    Account account;
    Posting posting;
    Transaction txn;

    char *cursor = file_buffer;

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

    print_ledger(l);
    printf("\n");
    print_file_buffer(file_buffer);

    ledger_free(l);
    free(file_buffer);

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
    free(file_buffer);
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

char *token_type_to_string(TokenType type) {
    switch (type) {
    case TOKEN_INVALID:
        return "INVALID";
    case TOKEN_DATE:
        return "DATE";
    case TOKEN_KEYWORD:
        return "KEYWORD";
    case TOKEN_ACCOUNT:
        return "ACCOUNT";
    case TOKEN_NUMBER:
        return "NUMBER";
    case TOKEN_CURRENCY:
        return "CURRENCY";
    case TOKEN_STRING:
        return "STRING";
    case TOKEN_ASTERISK:
        return "ASTERISK";
    case TOKEN_NEWLINE:
        return "NEWLINE";
    case TOKEN_INDENT:
        return "INDENT";
    case TOKEN_EOF:
        return "EOF";
    default:
        return "UNKNOWN";
    }
}

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

TokenArray *token_array_create(size_t capacity) {
    TokenArray *arr = malloc(sizeof(TokenArray));
    if (arr == NULL) {
        return NULL;
    }

    arr->data = malloc(capacity * sizeof(Token));
    if (arr->data == NULL) {
        free(arr);
        return NULL;
    }

    arr->size = 0;
    arr->capacity = capacity;
    return arr;
}

void token_array_free(TokenArray *arr) {
    if (arr == NULL) {
        return;
    }

    free(arr->data);
    free(arr);
}

void token_array_push(TokenArray *arr, Token value) {
    if (arr->size < arr->capacity) {
        arr->data[arr->size] = value;
        arr->size++;
        return;
    }

    size_t new_capacity = arr->capacity * 2;
    Token *new_data = realloc(arr->data, new_capacity * sizeof(Token));
    if (new_data == NULL) {
        return;
    }

    arr->data = new_data;
    arr->data[arr->size] = value;
    arr->size++;
    arr->capacity = new_capacity;
}

void print_token(Token token) {
    printf("%s", token_type_to_string(token.type));

    if (token.type == TOKEN_EOF) {
        printf("\n");
        return;
    }

    printf(" [line %zu]", token.line);

    if (token.type == TOKEN_NEWLINE) {
        printf("\n");
        return;
    }

    if (token.type == TOKEN_INDENT) {
        printf(" (size %zu)\n", token.text.len);
        return;
    }

    char token_text[MAX_TOKEN_LENGTH] = {0};
    slice_to_cstr(token.text, token_text, MAX_TOKEN_LENGTH);
    printf(" %s\n", token_text);
}

typedef struct {
    // Ownership: Buffer is not owned by Scanner and needs to be free by caller
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

void scanner_skip_comments(Scanner *s) {
    char c = *s->current;

    // Consume multiple lines of consecutive comments
    while (c == ';') {
        while (c != '\n' && c != '\0') {
            s->current++;
            c = *s->current;
        }

        // Check if next line is also a comment
        if (c == '\n') {
            const char *start = s->current;
            s->current++;
            scanner_skip_whitespace(s);
            c = *s->current;
            if (c == ';') {
                s->line++;
            } else {
                // Backtrack if not
                s->current = start;
                c = *s->current;
            }
        }
    }
}

bool try_scan_date(Scanner *s, Token *t) {
    const char *start = s->current;
    const char *peek = s->current;

    // Consume year (YYYY)
    for (int i = 0; i < 3; ++i) {
        peek++;
        if (!isdigit(*peek)) {
            return false;
        }
    }

    // Consume separator (-)
    peek++;
    if (*peek != '-') {
        return false;
    }

    // Consume month (MM)
    for (int i = 0; i < 2; ++i) {
        peek++;
        if (!isdigit(*peek)) {
            return false;
        }
    }

    // Consume separator (-)
    peek++;
    if (*peek != '-') {
        return false;
    }

    // Consume day (DD)
    for (int i = 0; i < 2; ++i) {
        peek++;
        if (!isdigit(*peek)) {
            return false;
        }
    }

    peek++;

    t->type = TOKEN_DATE;
    t->text = (StringSlice){
        .start = start,
        .len = (size_t)(peek - start),
    };
    t->line = s->line;

    s->current = peek;

    return true;
}

bool try_scan_number(Scanner *s, Token *t) {
    const char *start = s->current;
    const char *peek = s->current;

    // Consume optional negative sign (-)
    if (*peek == '-') {
        peek++;
    }

    // At least one digit
    if (!isdigit(*peek)) {
        return false;
    }
    peek++;

    // Consume rest of digits
    while (*peek != '\0' && isdigit(*peek)) {
        peek++;
    }

    // Optional decimal point (.)
    if (*peek == '.') {
        peek++;
        // At least one digit after decimal point
        if (!isdigit(*peek)) {
            return false;
        }
        peek++;
        // Consume rest of digits after decimal point
        while (*peek != '\0' && isdigit(*peek)) {
            peek++;
        }
    }

    t->type = TOKEN_NUMBER;
    t->text = (StringSlice){
        .start = start,
        .len = (size_t)(peek - start),
    };
    t->line = s->line;

    s->current = peek;

    return true;
}

Token scanner_next_token(Scanner *s) {

    scanner_skip_whitespace(s);
    scanner_skip_comments(s);

    char c = *s->current;
    Token t = {0};

    // EOF
    if (c == '\0') {
        t.type = TOKEN_EOF;
        return t;
    }

    // Newline
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

    // Asterisk
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

    // Keyword
    if (islower(c)) {
        const char *start = s->current;
        while (islower(c)) {
            s->current++;
            c = *s->current;
        }

        t.type = TOKEN_KEYWORD;
        t.text = (StringSlice){
            .start = start,
            .len = (size_t)(s->current - start),
        };
        t.line = s->line;
        return t;
    }

    // Currency or Account
    if (isupper(c)) {
        const char *start = s->current;
        bool all_uppercase = true;
        bool has_colon = false;
        while (isalnum(c) || c == ':') {
            if (islower(c)) {
                all_uppercase = false;
            } else if (c == ':') {
                has_colon = true;
            }

            s->current++;
            c = *s->current;
        }

        if (has_colon) {
            t.type = TOKEN_ACCOUNT;
        } else if (all_uppercase) {
            t.type = TOKEN_CURRENCY;
        } else {
            t.type = TOKEN_INVALID;
        }

        t.text = (StringSlice){
            .start = start,
            .len = (size_t)(s->current - start),
        };
        t.line = s->line;
        return t;
    }

    // String
    if (c == '\"') {
        const char *start = s->current;
        bool invalid = false;
        s->current++;
        c = *s->current;
        while (c != '\"') {
            if (c == '\0' || c == '\n') {
                invalid = true;
                break;
            }

            s->current++;
            c = *s->current;
        }

        if (invalid) {
            t.type = TOKEN_INVALID;
            t.text = (StringSlice){
                .start = start,
                .len = 1,
            };
            t.line = s->line;
            // Go back to just after the opening '\"'
            s->current = start + 1;
            return t;
        }

        // Consume closing '\"'
        s->current++;

        t.type = TOKEN_STRING;
        t.text = (StringSlice){
            .start = start,
            .len = (size_t)(s->current - start),
        };
        t.line = s->line;
        return t;
    }

    // Date or Number
    if (isdigit(c)) {
        if (try_scan_date(s, &t)) {
            return t;
        }

        if (try_scan_number(s, &t)) {
            return t;
        }
    } else if (c == '-') {
        if (try_scan_number(s, &t)) {
            return t;
        }
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

    free(file_buffer);

    return EXIT_SUCCESS;
}

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

Parser parser_init(TokenArray *tokens, Ledger *ledger) {
    assert(tokens->data[tokens->size - 1].type == TOKEN_EOF && "Tokens must end with EOF");

    Parser parser = {0};

    parser.tokens = tokens;
    parser.ledger = ledger;

    return parser;
}

Token *parser_current_token(Parser *p) {
    return &p->tokens->data[p->current];
}

Token *parser_peek(Parser *p) {
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

Token *parser_expect(Parser *p, TokenType token_type) {
    Token *t = parser_current_token(p);
    if (t->type != token_type) {
        p->has_error = true;
        (void)snprintf(p->error_message,
                       MAX_ERROR_MESSAGE_LENGTH,
                       "Expected token %s got %s",
                       token_type_to_string(token_type),
                       token_type_to_string(t->type));
        p->error_line = t->line;
        return NULL;
    }

    parser_advance(p);
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

    t = parser_current_token(p);
    if (t->type != TOKEN_NEWLINE && t->type != TOKEN_EOF) {
        p->has_error = true;
        (void)snprintf(p->error_message,
                       MAX_ERROR_MESSAGE_LENGTH,
                       "Expected newline or EOF after commodity directive");
        p->error_line = t->line;
        return;
    }
    if (t->type == TOKEN_NEWLINE) {
        parser_advance(p);
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

    t = parser_current_token(p);
    if (t->type != TOKEN_NEWLINE && t->type != TOKEN_EOF) {
        p->has_error = true;
        (void)snprintf(p->error_message,
                       MAX_ERROR_MESSAGE_LENGTH,
                       "Expected newline or EOF after open directive");
        p->error_line = t->line;
        return;
    }
    if (t->type == TOKEN_NEWLINE) {
        parser_advance(p);
    }

    account_array_push(l->accounts, account);
}

void parse_transaction(Parser *p) {
    Token *t;

    // Parse date
    t = parser_expect(p, TOKEN_DATE);
    if (p->has_error) {
        return;
    }
    Date date = parse_date(t->text);

    // Parse "txn" keyword, or a flag
    t = parser_current_token(p);
    Flag flag = FLAG_NONE;
    if (t->type == TOKEN_KEYWORD) {
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

    } else if (t->type == TOKEN_ASTERISK) {
        flag = FLAG_OKAY;
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
    parser_advance(p);

    // Parse optional payee and/or narration
    t = parser_expect(p, TOKEN_STRING);
    if (p->has_error) {
        return;
    }
    StringSlice payee = {0};
    StringSlice narration = t->text;
    t = parser_current_token(p);
    if (t->type == TOKEN_STRING) {
        payee = narration;
        narration = t->text;
    } else if (t->type != TOKEN_NEWLINE) {
        p->has_error = true;
        char buf[MAX_TOKEN_LENGTH] = {0};
        slice_to_cstr(t->text, buf, MAX_TOKEN_LENGTH);
        (void)snprintf(p->error_message,
                       MAX_ERROR_MESSAGE_LENGTH,
                       "Expected narration string or new line but got %s '%s'",
                       token_type_to_string(t->type),
                       buf);
        p->error_line = t->line;
        return;
    }
    parser_advance(p);

    Ledger *l = p->ledger;

    Transaction transaction = {0};
    transaction.date = date;
    transaction.flag = flag;
    transaction.payee = payee;
    transaction.narration = narration;

    transaction_array_push(l->transactions, transaction);
}

void parser_print_error(Parser *p) {
    if (!p->has_error) {
        return;
    }

    printf("Parse error: %s (line %zu)\n", p->error_message, p->error_line);
}

void parse_next(Parser *p) {
    Token *t = parser_current_token(p);
    if (t->type == TOKEN_EOF) {
        p->is_eof = true;
        return;
    }

    if (t->type == TOKEN_INVALID) {
        p->has_error = true;
        char token_buf[MAX_TOKEN_LENGTH] = {0};
        slice_to_cstr(t->text, token_buf, MAX_TOKEN_LENGTH);
        (void)snprintf(p->error_message, MAX_ERROR_MESSAGE_LENGTH, "Invalid token '%s'", token_buf);
        p->error_line = t->line;
        return;
    }

    bool matched = false;
    if (t->type == TOKEN_DATE) {
        Token *next = parser_peek(p);
        if (next->type == TOKEN_KEYWORD) {
            if (slice_equals_cstr(next->text, "commodity")) {
                parse_commodity_directive(p);
                matched = true;
            } else if (slice_equals_cstr(next->text, "open")) {
                parse_open_directive(p);
                matched = true;
            } else if (slice_equals_cstr(next->text, "txn")) {
                parse_transaction(p);
                matched = true;
            }
        } else if (next->type == TOKEN_ASTERISK) {
            parse_transaction(p);
            matched = true;
        }
    }

    if (!matched) {
        // TODO: this is probably not needed after all parse functions implemented
        // for now and testing, drop tokens
        parser_advance(p);
    }
}

int test_parser(char *ledger_path) {
    char *file_buffer = read_file(ledger_path);
    if (file_buffer == NULL) {
        printf("Error reading file\n");
        return EXIT_FAILURE;
    }

    Ledger *ledger = ledger_create(file_buffer);
    if (ledger == NULL) {
        printf("Error allocating ledger\n");
        free(file_buffer);
        return EXIT_FAILURE;
    }

    TokenArray *tokens = token_array_create(TOKEN_ARRAY_INITIAL_CAPACITY);
    if (tokens == NULL) {
        printf("Error allocating token array\n");
        free(file_buffer);
        ledger_free(ledger);
        return EXIT_FAILURE;
    }

    Scanner scanner = scanner_init(file_buffer);
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
            parser_print_error(&parser);
            break;
        }
    }

    print_ledger(parser.ledger);

    free(file_buffer);
    ledger_free(ledger);
    token_array_free(tokens);

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
        // return test_ledger();
        char *ledger_path = get_ledger_path(argc, argv);
        if (ledger_path == NULL) {
            return EXIT_FAILURE;
        }
        // return test_file(ledger_path);
        // return test_scanner(ledger_path);
        return test_parser(ledger_path);
    }

    printf("Unknown command: %s\n", command);
    printf("\n");
    print_usage();
    return EXIT_FAILURE;
}
