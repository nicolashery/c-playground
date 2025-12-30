#include "commands.h"
#include "ledger.h"
#include "parser.h"
#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage() {
    printf("Usage:\n\n");
    printf("  bean --help | -h            Print this text\n");
    printf("  bean check <ledger_file>    Parse and check ledger file\n");
    printf("  bean balance <ledger_file>  Compute per-account balances\n");
    printf("\n");
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
        if (argc < 3) {
            printf("Missing ledger file argument\n");
            printf("\n");
            print_usage();
            return EXIT_FAILURE;
        }
        return run_check(argv[2]);
    }

    if (strcmp(command, "balance") == 0) {
        if (argc < 3) {
            printf("Missing ledger file argument\n");
            printf("\n");
            print_usage();
            return EXIT_FAILURE;
        }
        return run_balance(argv[2]);
    }

    if (strcmp(command, "test") == 0) {
        if (argc < 3) {
            printf("Missing test component argument\n");
            printf("\n");
            return EXIT_FAILURE;
        }

        char *component = argv[2];

        if (strcmp(component, "ledger") == 0) {
            return test_ledger();
        }

        if (strcmp(component, "scanner") == 0) {
            if (argc < 4) {
                printf("Missing ledger file argument for test\n");
                printf("\n");
                return EXIT_FAILURE;
            }
            return test_scanner(argv[3]);
        }

        if (strcmp(component, "parser") == 0) {
            if (argc < 4) {
                printf("Missing ledger file argument for test\n");
                printf("\n");
                return EXIT_FAILURE;
            }
            return test_parser(argv[3]);
        }

        printf("Unknown test component: %s\n", component);
        printf("\n");
        return EXIT_FAILURE;
    }

    printf("Unknown command: %s\n", command);
    printf("\n");
    print_usage();
    return EXIT_FAILURE;
}
