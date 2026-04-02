#include "arena.h"

#include <assert.h>
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

static bool is_power_of_two(size_t alignment) {
    return alignment > 0 && (alignment & (alignment - 1)) == 0;
}

static void *arena_alloc_impl(Arena *arena, size_t size, size_t alignment, bool zero) {
    if (size == 0) {
        return NULL;
    }

    assert(is_power_of_two(alignment));

    // Round offset up to the next multiple of alignment
    size_t aligned_offset = (arena->offset + alignment - 1) & ~(alignment - 1);
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
    return arena_alloc_impl(arena, size, alignment, false);
}

void *arena_alloc_aligned(Arena *arena, size_t size, size_t alignment) {
    return arena_alloc_impl(arena, size, alignment, true);
}

void *arena_alloc_no_zero(Arena *arena, size_t size) {
    return arena_alloc_impl(arena, size, DEFAULT_ALIGNMENT, false);
}

void *arena_alloc(Arena *arena, size_t size) {
    return arena_alloc_aligned(arena, size, DEFAULT_ALIGNMENT);
}

TempArena temp_arena_begin(Arena *arena) {
    return (TempArena){
        .arena = arena,
        .offset = arena->offset,
    };
}

void temp_arena_end(TempArena temp) {
    temp.arena->offset = temp.offset;
}
