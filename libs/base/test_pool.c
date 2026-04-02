#include "unity.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_basic_allocation(void) {
    TEST_ASSERT_EQUAL_INT32(42, 42);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic_allocation);
    return UNITY_END();
}
