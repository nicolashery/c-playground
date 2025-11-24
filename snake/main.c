#include "foo.h"
#include "math_utils.h"

#include <stdio.h>

int main(void) {
    char *program_name = "snake";
    Foo_Message *msg = foo_new(program_name);
    for (int i = 0; i < 10; i++) {
        foo_print(msg);
    }

    double distance = distance_from_origin((Point_2D){
        .x = 10,
        .y = 20,
    });
    printf("Distance: %.3f\n", distance);
}
