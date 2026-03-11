#ifndef ARENA_H
#define ARENA_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    void *memory;
    size_t size;
    size_t offset;
} Arena;

Arena *arena_create(size_t size);

void arena_free(Arena *arena);

void arena_reset(Arena *arena);

void *arena_alloc_aligned_no_zero(Arena *arena, size_t size, size_t alignment);

void *arena_alloc_aligned(Arena *arena, size_t size, size_t alignment);

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT _Alignof(max_align_t)
#endif

void *arena_alloc_no_zero(Arena *arena, size_t size);

void *arena_alloc(Arena *arena, size_t size);

#define arena_push(a, T) (T *)arena_alloc_aligned(a, sizeof(T), _Alignof(T))
#define arena_push_array(a, T, n) (T *)arena_alloc_aligned(a, sizeof(T) * (n), _Alignof(T))

#endif
