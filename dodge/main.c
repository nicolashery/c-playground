#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>

#define PLAYER_WIDTH 100
#define PLAYER_HEIGHT 60
#define PLAYER_SPEED 240 // in pixels per second

#define BLOCK_SIZE 50
#define BLOCK_SPEED 240
#define MAX_BLOCKS 10
#define SPAWN_INTERVAL 2.0F // seconds between spawns

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
    bool active;
} Block;

void game_state_reset(Player *player, Block *blocks, float *time_since_last_spawn) {
    // Center at bottom of screen
    player->position = (Vector2){
        .x = (float)GetScreenWidth() / 2 - (float)PLAYER_WIDTH / 2,
        .y = (float)GetScreenHeight() - PLAYER_HEIGHT - 10,
    };

    // Center at top of screen
    blocks[0].position = (Vector2){
        .x = (float)GetScreenWidth() / 2 - (float)BLOCK_SIZE / 2,
        .y = 10,
    };
    blocks[0].active = true;

    *time_since_last_spawn = 0.0F;
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
    GameScreen screen = GAMEPLAY;

    Player player = {0};
    Block blocks[MAX_BLOCKS] = {0};
    float time_since_last_spawn = 0.0F;
    game_state_reset(&player, blocks, &time_since_last_spawn);

    // For debugging
    SetRandomSeed(123456);

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
            // Move player
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

            // Quit gameplay
            if (IsKeyPressed(KEY_Q)) {
                screen = ENDING;
            }

            // Move blocks
            for (size_t i = 0; i < MAX_BLOCKS; i++) {
                Block *block = &blocks[i];
                if (!block->active) {
                    continue;
                }

                bool block_off_screen = (block->position.y > (float)GetScreenHeight());
                if (block_off_screen) {
                    block->active = false;
                } else {
                    block->position.y += BLOCK_SPEED * GetFrameTime();
                }

                if (CheckCollisionRecs(player_bounds(&player), block_bounds(block))) {
                    screen = ENDING;
                }
            }

            // Spawn blocks
            time_since_last_spawn += GetFrameTime();
            if (time_since_last_spawn > SPAWN_INTERVAL) {
                time_since_last_spawn = 0.0F;

                Block *block = nullptr;
                for (size_t i = 0; i < MAX_BLOCKS; i++) {
                    if (blocks[i].active) {
                        continue;
                    }

                    block = &blocks[i];
                }

                if (block == nullptr) {
                    printf("[WARNING] Could not find a free block to spawn, consider increasing "
                           "MAX_BLOCKS (%d)",
                           MAX_BLOCKS);
                } else {
                    block->position = (Vector2){
                        .x = (float)GetRandomValue(0, GetScreenWidth() - BLOCK_SIZE),
                        .y = 10,
                    };
                    block->active = true;
                }
            }
        } break;
        case ENDING: {
            if (IsKeyPressed(KEY_ENTER)) {
                screen = GAMEPLAY;
                game_state_reset(&player, blocks, &time_since_last_spawn);
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

            for (size_t i = 0; i < MAX_BLOCKS; i++) {
                Block *block = &blocks[i];
                if (!block->active) {
                    continue;
                }

                DrawRectangle(
                    (int)block->position.x, (int)block->position.y, BLOCK_SIZE, BLOCK_SIZE, MAROON);
            }

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
