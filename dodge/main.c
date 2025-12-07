#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>

#define PLAYER_WIDTH 100
#define PLAYER_HEIGHT 60
#define PLAYER_SPEED 240 // in pixels per second

typedef enum {
    TITLE,
    GAMEPLAY,
    ENDING,
} GameScreen;

typedef struct {
    Vector2 position;
} Player;

void game_state_reset(Player *player) {
    // Center at bottom of screen
    player->position = (Vector2){
        .x = (float)GetScreenWidth() / 2 - (float)PLAYER_WIDTH / 2,
        .y = (float)GetScreenHeight() - PLAYER_HEIGHT - 10,
    };
}

int main(void) {
    // Initialization
    //---------------------------------------------------------------------------------------------
    const int screen_width = 800;
    const int screen_height = 450;

    InitWindow(screen_width, screen_height, "Dodge the Blocks");

    SetTargetFPS(120);

    // Game state
    GameScreen screen = TITLE;

    Player player = {0};
    game_state_reset(&player);

    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        //-----------------------------------------------------------------------------------------
        switch (screen) {
        case TITLE: {
            if (IsKeyPressed(KEY_ENTER)) {
                screen = GAMEPLAY;
            }

        } break;
        case GAMEPLAY: {
            if (IsKeyDown(KEY_RIGHT)) {
                player.position.x += (float)PLAYER_SPEED * GetFrameTime();
            }
            if (IsKeyDown(KEY_LEFT)) {
                player.position.x -= (float)PLAYER_SPEED * GetFrameTime();
            }

            if (IsKeyPressed(KEY_Q)) {
                screen = ENDING;
            }
        } break;
        case ENDING: {
            if (IsKeyPressed(KEY_ENTER)) {
                screen = GAMEPLAY;
                game_state_reset(&player);
            }
        } break;
        }

        // Draw
        //-----------------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (screen) {
        case TITLE: {
            DrawText("Dodge the Blocks", 20, 20, 40, DARKGREEN);
            char *start_text = "Press [ENTER] to start";
            DrawText(start_text,
                     GetScreenWidth() / 2 - MeasureText(start_text, 20) / 2,
                     GetScreenHeight() / 2 + 10,
                     20,
                     DARKGRAY);

        } break;
        case GAMEPLAY: {
            DrawFPS(GetScreenWidth() - 80, 10);
            DrawRectangle((int)player.position.x,
                          (int)player.position.y,
                          PLAYER_WIDTH,
                          PLAYER_HEIGHT,
                          DARKBLUE);
        } break;
        case ENDING: {
            DrawText("Game over!", 20, 20, 40, RED);
            char *play_again_text = "Press [ENTER] to play again";
            DrawText(play_again_text,
                     GetScreenWidth() / 2 - MeasureText(play_again_text, 20) / 2,
                     GetScreenHeight() / 2 + 10,
                     20,
                     DARKGRAY);
        } break;
        }

        EndDrawing();
    }

    // De-Initialization
    //---------------------------------------------------------------------------------------------
    CloseWindow();

    return EXIT_SUCCESS;
}
