#ifndef MATH_UTILS_H
#define MATH_UTILS_H

typedef struct {
    float x;
    float y;
} Point_2D;

float add_float(float a, float b);

float distance_from_origin(Point_2D point);

#endif
