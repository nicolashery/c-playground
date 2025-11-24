#include "math_utils.h"

#include <math.h>

double add_double(double a, double b) {
    return a + b;
}

double distance_from_origin(Point_2D point) {
    double squared = point.x * point.x + point.y * point.y;
    return sqrt(squared);
}
