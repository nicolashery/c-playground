#include "arena.h"
#include "strings.h"
#include "unity.h"

#include <string.h>

void setUp(void) {
}

void tearDown(void) {
}

void test_create(void) {
    String str = string_create("hello");

    TEST_ASSERT_EQUAL_size_t(5, str.length);
}

void test_slice(void) {
    String str = string_create("hello world");
    String str_slice = string_slice(str, 6, str.length);

    TEST_ASSERT_EQUAL_size_t(5, str_slice.length);
    TEST_ASSERT_EQUAL_CHAR(str_slice.data[0], 'w');
    TEST_ASSERT_EQUAL_CHAR(str_slice.data[str_slice.length - 1], 'd');

    String str_empty = string_create("");
    str_slice = string_slice(str_empty, 0, 0);
    TEST_ASSERT_EQUAL_size_t(0, str_slice.length);
}

void test_equals(void) {
    String str1 = string_create("hello");
    String str2 = string_create("hello");
    String str3 = string_create("world");
    String str_zero = string_create(NULL);

    TEST_ASSERT(string_equals(str1, str2));
    TEST_ASSERT(!string_equals(str1, str3));
    TEST_ASSERT(!string_equals(str1, str_zero));
    TEST_ASSERT(string_equals(str_zero, str_zero));
}

void test_cstring(void) {
    Arena *arena = arena_create(64);
    String str = string_create("hello");
    char *cstr = string_to_cstr(arena, str);

    TEST_ASSERT_EQUAL_size_t(5, strlen(cstr));

    arena_free(arena);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create);
    RUN_TEST(test_slice);
    RUN_TEST(test_equals);
    RUN_TEST(test_cstring);
    return UNITY_END();
}
