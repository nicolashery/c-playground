#include "arena.h"
#include "unity.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void setUp(void) {
}

void tearDown(void) {
}

void test_basic_allocation(void) {
    Arena *arena = arena_create(64);
    int32_t *p = arena_push(arena, int32_t);
    *p = 42;
    TEST_ASSERT_EQUAL_INT32(42, *p);
    arena_free(arena);
}

void test_alignment(void) {
    Arena *arena = arena_create(64);
    // 1 byte (at offset 0)
    (void)arena_push(arena, char);
    // 4 bytes (expect at offset 4, not 1)
    int32_t *p = arena_push(arena, int32_t);
    // new offset should be 4 (aligned offset) + 4 (bytes allocated)
    TEST_ASSERT_EQUAL_size_t(8, arena->offset);
    // and address should be 4-byte aligned
    TEST_ASSERT_EQUAL_INT(0, (uintptr_t)p % 4);
    arena_free(arena);
}

void test_out_of_space(void) {
    Arena *arena = arena_create(8);
    char *p = arena_push_array(arena, char, 16);
    TEST_ASSERT_EQUAL_PTR(NULL, p);
    arena_free(arena);
}

void test_reset(void) {
    Arena *arena = arena_create(64);
    int32_t *p1 = arena_push(arena, int32_t);
    arena_reset(arena);
    int32_t *p2 = arena_push(arena, int32_t);
    TEST_ASSERT_EQUAL_PTR(p1, p2);
    arena_free(arena);
}

typedef struct {
    int age;
    bool likes_pets;
} Person;

void test_zero_initialization(void) {
    Arena *arena = arena_create(64);
    Person *p = arena_push(arena, Person);
    TEST_ASSERT_EQUAL_INT(0, p->age);
    TEST_ASSERT_EQUAL(false, p->likes_pets);
    arena_free(arena);
}

void test_temp_arena(void) {
    Arena *arena = arena_create(64);
    arena_push(arena, int32_t);
    TEST_ASSERT_EQUAL_size_t(4, arena->offset);
    TempArena temp = temp_arena_begin(arena);
    arena_push_array(arena, char, 8);
    temp_arena_end(temp);
    TEST_ASSERT_EQUAL_size_t(4, arena->offset);
    arena_free(arena);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic_allocation);
    RUN_TEST(test_alignment);
    RUN_TEST(test_out_of_space);
    RUN_TEST(test_reset);
    RUN_TEST(test_zero_initialization);
    RUN_TEST(test_temp_arena);
    return UNITY_END();
}
