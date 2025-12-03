#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float radius;
    Vector2 position;
} CircleState;

int main(void) {
    const int screen_width = 800;
    const int screen_height = 450;

    InitWindow(screen_width, screen_height, "Dodge the Blocks");

    SetTargetFPS(60);

    CircleState circle = {
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

    return EXIT_SUCCESS;
}
