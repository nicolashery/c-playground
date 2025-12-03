#include "math_utils.h"

#include <math.h>

float add_float(float a, float b) {
    return a + b;
}

float distance_from_origin(Point2D point) {
    float squared = point.x * point.x + point.y * point.y;
    return sqrtf(squared);
}
