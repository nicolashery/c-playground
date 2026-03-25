#include "arena.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int age;
    bool likes_pets;
} Person;

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

    // macro usage
    (void)arena_push(arena, Person);
    (void)arena_push(arena, double);
    (void)arena_push_array(arena, char, 8);

    arena_reset(arena);

    // temp arena usage
    arena_push(arena, int32_t);
    TempArena temp = temp_arena_begin(arena);
    arena_push_array(arena, char, 8);
    temp_arena_end(temp);

    arena_free(arena);

    return EXIT_SUCCESS;
}
