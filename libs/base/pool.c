#include "pool.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static bool is_power_of_two(size_t alignment) {
    return alignment > 0 && (alignment & (alignment - 1)) == 0;
}

void pool_reset(Pool *pool) {
    PoolFreeNode *head = NULL;
    for (size_t i = 0; i < pool->block_count; i++) {
        char *p = (char *)pool->memory + (i * pool->block_size);
        PoolFreeNode *node = (PoolFreeNode *)p;
        node->next = head;
        head = node;
    }

    pool->head = head;
}

Pool *pool_create_aligned(size_t block_size, size_t block_count, size_t block_alignment) {
    assert(block_size >= sizeof(PoolFreeNode) &&
           "block size must be large enough to hold free list node");
    assert(is_power_of_two(block_alignment));

    Pool *pool = malloc(sizeof(Pool));
    if (pool == NULL) {
        return NULL;
    }

    // Round block size up to the next multiple of block alignment
    size_t aligned_block_size = (block_size + block_alignment - 1) & ~(block_alignment - 1);
    pool->block_size = aligned_block_size;

    if (block_count == 0) {
        free(pool);
        return NULL;
    }
    pool->block_count = block_count;

    void *memory = malloc(pool->block_size * pool->block_count);
    if (memory == NULL) {
        free(pool);
        return NULL;
    }
    pool->memory = memory;

    pool->head = NULL;
    pool_reset(pool);

    return pool;
}

static void *pool_alloc_impl(Pool *pool, bool zero) {
    if (pool->head == NULL) {
        return NULL;
    }

    void *result = pool->head;
    pool->head = pool->head->next;

    if (zero) {
        memset(result, 0, pool->block_size);
    }

    return result;
}

void *pool_alloc_no_zero(Pool *pool) {
    return pool_alloc_impl(pool, false);
}

void *pool_alloc(Pool *pool) {
    return pool_alloc_impl(pool, true);
}

void pool_free(Pool *pool, void *ptr) {
    if (ptr == NULL) {
        return;
    }

    void *pool_end = (char *)pool->memory + (pool->block_size * pool->block_count);
    assert(pool->memory <= ptr && ptr < pool_end && "ptr failed pool memory bounds check");

    // note that casting to (unsigned) size_t is safe here because we know ptr >= memory from
    // previous assert (so we know ptrdiff_t is non-negative)
    size_t relative_offset = (size_t)((char *)ptr - (char *)pool->memory);
    assert((relative_offset % pool->block_size) == 0 && "ptr should point to start of a block");

    PoolFreeNode *previous_head = pool->head;
    pool->head = (PoolFreeNode *)ptr;
    pool->head->next = previous_head;
}
