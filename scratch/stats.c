#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} Int_Array;

Int_Array *int_array_create(size_t capacity) {
    int *data = malloc(capacity * sizeof(int));
    if (data == nullptr) {
        return nullptr;
    }

    Int_Array *arr = malloc(sizeof(Int_Array));
    if (arr == nullptr) {
        free(data);
        return nullptr;
    }

    arr->data = data;
    arr->size = 0;
    arr->capacity = capacity;
    return arr;
}

void int_array_append(Int_Array *arr, int value) {
    if (arr->size < arr->capacity) {
        arr->data[arr->size] = value;
        arr->size++;
        return;
    }

    size_t new_capacity = arr->capacity * 2;
    int *new_data = realloc(arr->data, new_capacity * sizeof(int));
    if (new_data == nullptr) {
        printf("WARNING: Failed to realloc Int_Array with new capacity %zu, dropping value %d\n",
               new_capacity,
               value);
        return;
    }

    arr->data = new_data;
    printf("DEBUG: Reallocated Int_Array with new capacity %zu\n", new_capacity);

    arr->data[arr->size] = value;
    arr->size++;
    arr->capacity = new_capacity;
}

void int_array_free(Int_Array *arr) {
    free(arr->data);
    free(arr);
}

int min(Int_Array *arr) {
    assert(arr->size > 0 && "Array must not be empty");

    int result = arr->data[0];
    for (size_t i = 1; i < arr->size; i++) {
        if (arr->data[i] < result) {
            result = arr->data[i];
        }
    }
    return result;
}

int max(Int_Array *arr) {
    assert(arr->size > 0 && "Array must not be empty");

    int result = arr->data[0];
    for (size_t i = 1; i < arr->size; i++) {
        if (arr->data[i] > result) {
            result = arr->data[i];
        }
    }
    return result;
}

double average(Int_Array *arr) {
    assert(arr->size > 0 && "Array must not be empty");

    int sum = 0;
    for (size_t i = 0; i < arr->size; i++) {
        sum += arr->data[i];
    }

    return (sum / (double)arr->size);
}

int stats_main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("ERROR: Must provide a file name\n");
        return EXIT_FAILURE;
    }

    char *file_name = argv[1];
    FILE *file = fopen(file_name, "r");
    if (file == nullptr) {
        printf("ERROR: Could not open file: %s\n", file_name);
        return EXIT_FAILURE;
    }

    // Allocating small size (8 ints) on purpose to demonstrate reallocation
    Int_Array *arr = int_array_create(8);
    if (arr == nullptr) {
        printf("ERROR: Failed to allocate Int_Array\n");
        (void)fclose(file);
        return EXIT_FAILURE;
    }

    int num;
    int count = 0;
    // No need to use `strtol` for this simple exercise
    // NOLINTNEXTLINE(cert-err34-c)
    while (fscanf(file, "%d", &num) == 1) {
        count++;
        int_array_append(arr, num);
    }
    if (count == 0) {
        printf("WARNING: No numbers found in file, aborting.\n");
        int_array_free(arr);
        (void)fclose(file);
        return EXIT_FAILURE;
    }

    printf("\n");
    printf("Count: %d\n", count);
    printf("Min: %d\n", min(arr));
    printf("Max: %d\n", max(arr));
    printf("Average: %.3f\n", average(arr));

    int_array_free(arr);
    (void)fclose(file);

    return EXIT_SUCCESS;
}
