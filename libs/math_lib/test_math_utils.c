#include "math_utils.h"
#include "unity.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_add_float(void) {
    float result1 = add_float(2.0F, 3.0F);
    TEST_ASSERT_EQUAL_FLOAT(5.0F, result1);

    float result2 = add_float(-1.0F, 1.0F);
    TEST_ASSERT_EQUAL_FLOAT(0.0F, result2);

    float result3 = add_float(-2.0F, -3.0F);
    TEST_ASSERT_EQUAL_FLOAT(-5.0F, result3);
}

void test_distance_from_origin(void) {
    Point2D point1 = {3.0F, 4.0F};
    float result1 = distance_from_origin(point1);
    TEST_ASSERT_EQUAL_FLOAT(5.0F, result1);

    Point2D point2 = {0.0F, 0.0F};
    float result2 = distance_from_origin(point2);
    TEST_ASSERT_EQUAL_FLOAT(0.0F, result2);

    Point2D point3 = {1.0F, 1.0F};
    float result3 = distance_from_origin(point3);
    TEST_ASSERT_FLOAT_WITHIN(0.001F, 1.414F, result3);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_add_float);
    RUN_TEST(test_distance_from_origin);
    return UNITY_END();
}
