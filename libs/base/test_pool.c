#include "pool.h"
#include "unity.h"

void setUp(void) {
}

void tearDown(void) {
}

typedef struct {
    int x;
    int y;
} Point;

void test_basic_allocation(void) {
    Pool *pool = pool_create(Point, 4);
    Point *p = pool_alloc(pool);
    p->x = 42;
    TEST_ASSERT_EQUAL_INT32(42, p->x);
    pool_destroy(pool);
}

void test_alignment(void) {
    Pool *pool = pool_create_aligned(10, 4, 4);
    // requested size 10 and alignment 4
    // resulting block size should be 12 (nearest multiple of 4 above 10)
    TEST_ASSERT_EQUAL_size_t(12, pool->block_size);
    (void)pool_alloc(pool);
    void *p2 = pool_alloc(pool);
    // addresses should be 4-byte aligned
    TEST_ASSERT_EQUAL_size_t(0, (uintptr_t)p2 % 4);
    pool_destroy(pool);
}

void test_out_of_space(void) {
    Pool *pool = pool_create(Point, 2);
    Point *p1 = pool_alloc(pool);
    Point *p2 = pool_alloc(pool);
    Point *p3 = pool_alloc(pool);
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT_NULL(p3);
    pool_destroy(pool);
}

void test_reuse(void) {
    Pool *pool = pool_create(Point, 2);
    Point *p1 = pool_alloc(pool);
    (void)pool_alloc(pool);
    pool_free(pool, p1);
    Point *p3 = pool_alloc(pool);
    TEST_ASSERT_NOT_NULL(p3);
    pool_destroy(pool);
}

void test_zero_initialization(void) {
    Pool *pool = pool_create(Point, 4);
    Point *p = pool_alloc(pool);
    TEST_ASSERT_EQUAL_INT32(0, p->x);
    TEST_ASSERT_EQUAL_INT32(0, p->y);
    pool_destroy(pool);
}

void test_reset(void) {
    Pool *pool = pool_create(Point, 2);
    (void)pool_alloc(pool);
    (void)pool_alloc(pool);
    pool_reset(pool);
    Point *p3 = pool_alloc(pool);
    TEST_ASSERT_NOT_NULL(p3);
    pool_destroy(pool);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic_allocation);
    RUN_TEST(test_alignment);
    RUN_TEST(test_out_of_space);
    RUN_TEST(test_reuse);
    RUN_TEST(test_zero_initialization);
    RUN_TEST(test_reset);
    return UNITY_END();
}
