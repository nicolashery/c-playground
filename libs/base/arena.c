#include "arena.h"

#include <stdlib.h>

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

void *arena_alloc(Arena *arena, size_t size) {
    if (size == 0) {
        return NULL;
    }

    size_t new_offset = arena->offset + size;
    if (new_offset > arena->size) {
        return NULL;
    }

    char *p = (char *)arena->memory + arena->offset;

    arena->offset = new_offset;

    return p;
}

void *arena_alloc_aligned(Arena *arena, size_t size, size_t alignment) {
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

    arena->offset = new_offset;

    return p;
}
