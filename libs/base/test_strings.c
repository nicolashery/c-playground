#include "strings.h"
#include "unity.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_dummy(void) {
    String str = string_create("hello");
    TEST_ASSERT_EQUAL_size_t(5, str.length);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_dummy);
    return UNITY_END();
}
