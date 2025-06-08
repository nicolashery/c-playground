#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: bean <ledger_path>\n");
        return EXIT_FAILURE;
    }

    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    char *ledger_path = argv[1];
    FILE *ledger_file = fopen(ledger_path, "rb");
    if (ledger_file == NULL) {
        printf("Error opening file: %s\n", ledger_path);
        return EXIT_FAILURE;
    }

    fseek(ledger_file, 0, SEEK_END);
    long fledger_size = ftell(ledger_file);
    if (fledger_size < 0) {
        printf("Error reading size of file: %s\n", ledger_path);
        fclose(ledger_file);
        return EXIT_FAILURE;
    }
    size_t ledger_size = (size_t)fledger_size;
    rewind(ledger_file);
    printf("[debug] Ledger file size (bytes): %ld\n", ledger_size);

    size_t null_terminator_size = 1;
    char *ledger_buffer = malloc(ledger_size + null_terminator_size);
    if (!ledger_buffer) {
        printf("Error allocating memory for reading ledger (bytes): %ld\n", ledger_size + null_terminator_size);
        fclose(ledger_file);
        return EXIT_FAILURE;
    }

    size_t bytes_read = fread(ledger_buffer, 1, ledger_size, ledger_file);
    if (bytes_read != ledger_size) {
        if (feof(ledger_file)) {
            printf("Error reached end-of-file before reading %ld bytes from file: %s\n", ledger_size, ledger_path);
        } else if (ferror(ledger_file)) {
            printf("Error reading file: %s\n", ledger_path);
        }

        free(ledger_buffer);
        fclose(ledger_file);
        return EXIT_FAILURE;
    }
    ledger_buffer[ledger_size] = '\0';

    int line_count = 0;
    for (size_t i = 0; i < ledger_size; ++i) {
        if (ledger_buffer[i] == '\n') {
            line_count++;
        }
    }
    printf("[debug] Ledger file size (lines): %d\n", line_count);

    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed_ms = (end_time.tv_sec - start_time.tv_sec)*1.0e3 +
        (end_time.tv_nsec - start_time.tv_nsec)/1.0e6;
    printf("[debug] Processing took: %.3f ms\n", elapsed_ms);

    free(ledger_buffer);
    fclose(ledger_file);

    return EXIT_SUCCESS;
}
