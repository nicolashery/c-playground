#include <stdio.h>
#include <stdlib.h>

void print_usage() {
    printf("Usage:\n\n");
    printf("  bean --help                 Print this text\n");
    printf("  bean check <ledger_file>    Parse and check ledger file\n");
    printf("  bean balance <ledger_file>  Compute per-account balances\n");
    printf("\n");
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    print_usage();

    return EXIT_SUCCESS;
}
