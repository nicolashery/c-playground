#ifndef MATH_UTILS_H
#define MATH_UTILS_H

typedef struct {
    float x;
    float y;
} Point2D;

float add_float(float a, float b);

float distance_from_origin(Point2D point);

#endif
