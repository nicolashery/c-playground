#include "arena.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

Arena *arena_create(size_t size) {
    Arena *arena = malloc(sizeof(Arena));
    if (arena == NULL) {
        return NULL;
    }

    arena->memory = malloc(size);
    if (arena->memory == NULL) {
        free(arena);
        return NULL;
    }

    arena->size = size;
    arena->offset = 0;

    return arena;
}

void arena_free(Arena *arena) {
    if (arena == NULL) {
        return;
    }

    if (arena->memory != NULL) {
        free(arena->memory);
    }

    free(arena);
}

void arena_reset(Arena *arena) {
    arena->offset = 0;
}

static void *arena_push(Arena *arena, size_t size, size_t alignment, bool zero) {
    if (size == 0) {
        return NULL;
    }

    size_t padding = (alignment - (arena->offset % alignment)) % alignment;
    size_t aligned_offset = arena->offset + padding;
    size_t new_offset = aligned_offset + size;
    if (new_offset > arena->size) {
        return NULL;
    }

    char *p = (char *)arena->memory + aligned_offset;
    if (zero) {
        memset(p, 0, size);
    }

    arena->offset = new_offset;

    return p;
}

void *arena_alloc_aligned_no_zero(Arena *arena, size_t size, size_t alignment) {
    return arena_push(arena, size, alignment, false);
}

void *arena_alloc_aligned(Arena *arena, size_t size, size_t alignment) {
    return arena_push(arena, size, alignment, true);
}

void *arena_alloc_no_zero(Arena *arena, size_t size) {
    return arena_push(arena, size, _Alignof(max_align_t), false);
}

void *arena_alloc(Arena *arena, size_t size) {
    return arena_alloc_aligned(arena, size, _Alignof(max_align_t));
}
