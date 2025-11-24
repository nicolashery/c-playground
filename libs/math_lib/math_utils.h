#ifndef MATH_UTILS_H
#define MATH_UTILS_H

typedef struct {
    double x;
    double y;
} Point_2D;

double add_double(double a, double b);

double distance_from_origin(Point_2D point);

#endif
