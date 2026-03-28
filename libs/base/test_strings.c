#include "strings.h"
#include "unity.h"

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
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create);
    RUN_TEST(test_slice);
    return UNITY_END();
}
