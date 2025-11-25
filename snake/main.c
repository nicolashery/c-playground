#include "foo.h"
#include "math_utils.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include <stdio.h>

typedef struct {
    char *key;
    int value;
} Int_Hash_Map;

void demo_hash_map(void) {
    Int_Hash_Map *map = NULL;
    sh_new_arena(map);

    shput(map, "foo", 1);
    shput(map, "bar", 2);

    int foo_value = shget(map, "foo");
    int bar_value = shget(map, "bar");

    printf("stb_ds demo: foo=%d, bar=%d\n", foo_value, bar_value);

    shfree(map);
}

int main(void) {
    char *program_name = "snake";
    Foo_Message *msg = foo_new(program_name);
    for (int i = 0; i < 10; i++) {
        foo_print(msg);
    }
    printf("\n");

    double distance = distance_from_origin((Point_2D){
        .x = 10,
        .y = 20,
    });
    printf("Distance: %.3f\n", distance);
    printf("\n");

    demo_hash_map();
    printf("\n");
}
