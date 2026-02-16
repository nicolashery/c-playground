#include "math_utils.h"

#include <stdlib.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include <stdio.h>

typedef struct {
    char *key;
    int value;
} IntHashMap;

void demo_hash_map(void) {
    IntHashMap *map = NULL;
    sh_new_arena(map);

    shput(map, "foo", 1);
    shput(map, "bar", 2);

    int foo_value = shget(map, "foo");
    int bar_value = shget(map, "bar");

    printf("stb_ds demo: foo=%d, bar=%d\n", foo_value, bar_value);

    shfree(map);
}

int stb_demo_main(void) {
    double distance = distance_from_origin((Point2D){
        .x = 10,
        .y = 20,
    });
    printf("Distance: %.3f\n", distance);
    printf("\n");

    demo_hash_map();

    return EXIT_SUCCESS;
}
