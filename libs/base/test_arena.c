#include "arena.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("%zu\n", _Alignof(max_align_t));

    Arena *arena = arena_create(32);

    // 1 byte
    // at offset 0
    char *p1 = arena_alloc_aligned(arena, sizeof(char), _Alignof(char));
    if (p1 != NULL) {
        *p1 = 'a';
    }

    // 4 bytes
    // but expect them at offset 4 (not 1)
    int32_t *p2 = arena_alloc_aligned(arena, sizeof(int32_t), _Alignof(int32_t));
    if (p2 != NULL) {
        *p2 = 42;
    }

    // 1 byte
    // at offset 8
    p1 = arena_alloc_aligned(arena, sizeof(char), _Alignof(char));
    if (p1 != NULL) {
        *p1 = 'b';
    }

    // 8 bytes
    // but expect them at offset 16 (not 9)
    int64_t *p3 = arena_alloc_aligned(arena, sizeof(int64_t), _Alignof(int64_t));
    if (p3 != NULL) {
        *p3 = 128;
    }

    arena_reset(arena);

    arena_free(arena);

    return EXIT_SUCCESS;
}
