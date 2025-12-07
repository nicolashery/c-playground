#include "raylib.h"

#include <stdlib.h>

#define PLAYER_WIDTH 100
#define PLAYER_HEIGHT 60
#define PLAYER_SPEED 240 // in pixels per second

#define BLOCK_SIZE 50

typedef enum {
    TITLE,
    GAMEPLAY,
    ENDING,
} GameScreen;

typedef struct {
    Vector2 position;
} Player;

typedef struct {
    Vector2 position;
    float speed;
} Block;

void game_state_reset(Player *player, Block *block) {
    // Center at bottom of screen
    player->position = (Vector2){
        .x = (float)GetScreenWidth() / 2 - (float)PLAYER_WIDTH / 2,
        .y = (float)GetScreenHeight() - PLAYER_HEIGHT - 10,
    };

    // Center at top of screen
    block->position = (Vector2){
        .x = (float)GetScreenWidth() / 2 - (float)BLOCK_SIZE / 2,
        .y = 10,
    };
    block->speed = 300;
}

float clamp(float value, float min, float max) {
    if (value < min) {
        return min;
    }

    if (value > max) {
        return max;
    }

    return value;
}

Rectangle player_bounds(Player *player) {
    return (Rectangle){
        .x = player->position.x,
        .y = player->position.y,
        .width = PLAYER_WIDTH,
        .height = PLAYER_HEIGHT,
    };
}

Rectangle block_bounds(Block *block) {
    return (Rectangle){
        .x = block->position.x,
        .y = block->position.y,
        .width = BLOCK_SIZE,
        .height = BLOCK_SIZE,
    };
}

int main(void) {
    // Initialization
    //---------------------------------------------------------------------------------------------
    InitWindow(800, 450, "Dodge the Blocks");

    SetTargetFPS(60);

    // Game state
    GameScreen screen = TITLE;

    Player player = {0};
    Block block = {0};
    game_state_reset(&player, &block);

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
                player.position.x = clamp(player.position.x + (float)PLAYER_SPEED * GetFrameTime(),
                                          0.0F,
                                          (float)GetScreenWidth() - PLAYER_WIDTH);
            }
            if (IsKeyDown(KEY_LEFT)) {
                player.position.x = clamp(player.position.x - (float)PLAYER_SPEED * GetFrameTime(),
                                          0.0F,
                                          (float)GetScreenWidth() - PLAYER_WIDTH);
            }

            if (IsKeyPressed(KEY_Q)) {
                screen = ENDING;
            }

            bool block_off_screen = (block.position.y > (float)GetScreenHeight());
            if (block_off_screen) {
                block.position.y = 10;
            } else {
                block.position.y += block.speed * GetFrameTime();
            }

            if (CheckCollisionRecs(player_bounds(&player), block_bounds(&block))) {
                screen = ENDING;
            }
        } break;
        case ENDING: {
            if (IsKeyPressed(KEY_ENTER)) {
                screen = GAMEPLAY;
                game_state_reset(&player, &block);
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
            const char *start_text = "Press [ENTER] to start";
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
            DrawRectangle(
                (int)block.position.x, (int)block.position.y, BLOCK_SIZE, BLOCK_SIZE, MAROON);
        } break;
        case ENDING: {
            DrawText("Game over!", 20, 20, 40, RED);
            const char *play_again_text = "Press [ENTER] to play again";
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
