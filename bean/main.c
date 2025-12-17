#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage() {
    printf("Usage:\n\n");
    printf("  bean --help|-h              Print this text\n");
    printf("  bean check <ledger_file>    Parse and check ledger file\n");
    printf("  bean balance <ledger_file>  Compute per-account balances\n");
    printf("\n");
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

    printf("Unknown command: %s\n", command);
    printf("\n");
    print_usage();
    return EXIT_FAILURE;
}
