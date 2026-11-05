#include "raylib.h"

#include <stdlib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 450

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Particle Simulation");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        const char *hello_world_text = "Hello world!";
        int hello_world_font_size = 26;
        DrawText(hello_world_text,
                 GetScreenWidth() / 2 - MeasureText(hello_world_text, hello_world_font_size) / 2,
                 GetScreenHeight() / 2 - hello_world_font_size,
                 hello_world_font_size,
                 DARKGREEN);

        EndDrawing();
    }

    CloseWindow();

    return EXIT_SUCCESS;
}
