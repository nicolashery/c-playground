#include "foo.h"
#include "math_utils.h"
#include "raylib.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include <stdio.h>

typedef struct {
    char *key;
    int value;
} Int_Hash_Map;

void demo_hash_map(void) {
    Int_Hash_Map *map = nullptr;
    sh_new_arena(map);

    shput(map, "foo", 1);
    shput(map, "bar", 2);

    int foo_value = shget(map, "foo");
    int bar_value = shget(map, "bar");

    printf("stb_ds demo: foo=%d, bar=%d\n", foo_value, bar_value);

    shfree(map);
}

typedef struct {
    float radius;
    Vector2 position;
} Circle_State;

void demo_raylib(void) {
    const int screen_width = 800;
    const int screen_height = 450;

    InitWindow(screen_width, screen_height, "Snake");

    SetTargetFPS(60);

    Circle_State circle = {
        .radius = 40.0F,
        .position =
            {
                .x = 400.0F,
                .y = 225.0F,
            },
    };

    while (!WindowShouldClose()) {
        // Update
        if (IsKeyDown(KEY_RIGHT)) {
            circle.position.x += 2.0F;
        }
        if (IsKeyDown(KEY_LEFT)) {
            circle.position.x -= 2.0F;
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Move the circle with left/right arrows", 10, 10, 20, DARKGRAY);
        DrawCircleV(circle.position, circle.radius, SKYBLUE);

        EndDrawing();
    }

    CloseWindow();
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

    demo_raylib();
}
