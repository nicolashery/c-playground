#ifndef POOL_H
#define POOL_H

#include <stddef.h>

typedef struct PoolFreeNode PoolFreeNode;
struct PoolFreeNode {
    PoolFreeNode *next;
};

typedef struct {
    void *memory;
    size_t block_size;
    size_t block_count;
    PoolFreeNode *head;
} Pool;

void pool_reset(Pool *p);

Pool *pool_create_aligned(size_t block_size, size_t block_count, size_t block_alignment);

#define pool_create(T, n) pool_create_aligned(sizeof(T), (n), _Alignof(T))

void *pool_alloc_no_zero(Pool *pool);

void *pool_alloc(Pool *pool);

void pool_free(Pool *pool, void *ptr);

#endif
