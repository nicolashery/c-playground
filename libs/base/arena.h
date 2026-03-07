#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

typedef struct {
    void *memory;
    size_t size;
    size_t offset;
} Arena;

Arena *arena_create(size_t size);

void arena_free(Arena *arena);

void arena_reset(Arena *arena);

void *arena_alloc(Arena *arena, size_t size);

void *arena_alloc_aligned(Arena *arena, size_t size, size_t alignment);

#endif
