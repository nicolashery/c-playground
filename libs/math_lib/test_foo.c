#include "unity.h"
#include "unity_internals.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_foo(void) {
    TEST_ASSERT_EQUAL_INT(42, 42);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_foo);
    return UNITY_END();
}
