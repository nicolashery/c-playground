#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>

#define PLAYER_WIDTH 100
#define PLAYER_HEIGHT 60
#define PLAYER_SPEED 240.0F // pixels per second

#define MAX_BLOCKS 32
#define BLOCK_SIZE 50
#define BLOCK_SPEED_INITIAL 120.0F         // pixels per second
#define BLOCK_SPEED_LEVEL_INCREASE 0.1F    // percent increase per level
#define SPAWN_INTERVAL_INITIAL 3.0F        // seconds between spawns
#define SPAWN_INTERVAL_LEVEL_DECREASE 0.2F // percent decrease per level
#define SCORE_POINTS_PER_BLOCK 50
#define LEVEL_CHANGE_INTERVAL 5.0F // seconds of survival between levels
#define MAX_LEVEL 10

#define TEXT_BUFFER_LENGTH 1024

#define DEBUG_INVINCIBLE false

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

typedef struct {
    Player player;
    Block blocks[MAX_BLOCKS];
    float time_since_last_spawn;
    float time_since_last_level;
    int score;
    int level;
    float block_speed;
    float spawn_interval;
} GameState;

void game_state_reset(GameState *state) {
    // Center at bottom of screen
    state->player.position = (Vector2){
        .x = (float)GetScreenWidth() / 2 - (float)PLAYER_WIDTH / 2,
        .y = (float)GetScreenHeight() - PLAYER_HEIGHT - 10,
    };

    for (size_t i = 0; i < MAX_BLOCKS; i++) {
        state->blocks[i].active = false;
    }
    // Center at top of screen
    state->blocks[0].position = (Vector2){
        .x = (float)GetScreenWidth() / 2 - (float)BLOCK_SIZE / 2,
        .y = 10,
    };
    state->blocks[0].active = true;

    state->time_since_last_spawn = 0.0F;
    state->time_since_last_level = 0.0F;

    state->score = 0;
    state->level = 1;
    state->block_speed = BLOCK_SPEED_INITIAL;
    state->spawn_interval = SPAWN_INTERVAL_INITIAL;
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

    InitAudioDevice();
    Sound fx_spawn = LoadSound("assets/laserSmall_000.ogg");
    Sound fx_collision = LoadSound("assets/explosionCrunch_003.ogg");

    // Game state
    GameScreen screen = TITLE;
    GameState state = {0};
    game_state_reset(&state);

    // Text buffers
    char score_text_buffer[TEXT_BUFFER_LENGTH] = {};
    char level_text_buffer[TEXT_BUFFER_LENGTH] = {};

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

                PlaySound(fx_spawn);
            }

        } break;
        case GAMEPLAY: {
            Player *player = &state.player;
            // Move player
            if (IsKeyDown(KEY_RIGHT)) {
                player->position.x = clamp(player->position.x + PLAYER_SPEED * GetFrameTime(),
                                           0.0F,
                                           (float)GetScreenWidth() - PLAYER_WIDTH);
            }
            if (IsKeyDown(KEY_LEFT)) {
                player->position.x = clamp(player->position.x - PLAYER_SPEED * GetFrameTime(),
                                           0.0F,
                                           (float)GetScreenWidth() - PLAYER_WIDTH);
            }

            // Quit gameplay
            if (IsKeyPressed(KEY_Q)) {
                screen = ENDING;
            }

            // Move blocks & check collision
            for (size_t i = 0; i < MAX_BLOCKS; i++) {
                Block *block = &state.blocks[i];
                if (!block->active) {
                    continue;
                }

                bool block_off_screen = (block->position.y > (float)GetScreenHeight());
                if (block_off_screen) {
                    block->active = false;
                    state.score += SCORE_POINTS_PER_BLOCK;
                } else {
                    block->position.y += state.block_speed * GetFrameTime();
                }

                if (!DEBUG_INVINCIBLE) {
                    if (CheckCollisionRecs(player_bounds(player), block_bounds(block))) {
                        screen = ENDING;

                        PlaySound(fx_collision);
                    }
                }
            }

            // Change level
            state.time_since_last_level += GetFrameTime();
            if (state.time_since_last_level > LEVEL_CHANGE_INTERVAL && state.level <= MAX_LEVEL) {
                state.time_since_last_level = 0.0F;
                state.level += 1;

                state.block_speed = state.block_speed * (1.0F + BLOCK_SPEED_LEVEL_INCREASE);
                state.spawn_interval =
                    state.spawn_interval * (1.0F - SPAWN_INTERVAL_LEVEL_DECREASE);
            }

            // Spawn blocks
            state.time_since_last_spawn += GetFrameTime();
            if (state.time_since_last_spawn > state.spawn_interval) {
                state.time_since_last_spawn = 0.0F;

                // Find first available block in pool
                Block *block = nullptr;
                for (size_t i = 0; i < MAX_BLOCKS; i++) {
                    if (!state.blocks[i].active) {
                        block = &state.blocks[i];
                        break;
                    }
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

                    PlaySound(fx_spawn);
                }
            }
        } break;
        case ENDING: {
            if (IsKeyPressed(KEY_ENTER)) {
                screen = GAMEPLAY;
                game_state_reset(&state);
            }
        } break;
        }

        // Draw
        //-----------------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (screen) {
        case TITLE: {
            const char *title_text = "Dodge the Blocks";
            DrawText(title_text,
                     GetScreenWidth() / 2 - MeasureText(title_text, 40) / 2,
                     GetScreenHeight() / 2 - 40,
                     40,
                     DARKGREEN);
            const char *start_text = "Press [ENTER] to start";
            DrawText(start_text,
                     GetScreenWidth() / 2 - MeasureText(start_text, 20) / 2,
                     GetScreenHeight() / 2 + 20,
                     20,
                     DARKGRAY);

        } break;
        case GAMEPLAY: {
            Player *player = &state.player;
            DrawRectangle((int)player->position.x,
                          (int)player->position.y,
                          PLAYER_WIDTH,
                          PLAYER_HEIGHT,
                          DARKBLUE);

            for (size_t i = 0; i < MAX_BLOCKS; i++) {
                Block *block = &state.blocks[i];
                if (!block->active) {
                    continue;
                }

                DrawRectangle(
                    (int)block->position.x, (int)block->position.y, BLOCK_SIZE, BLOCK_SIZE, MAROON);
            }

            (void)snprintf(score_text_buffer, TEXT_BUFFER_LENGTH, "Score %d", state.score);
            DrawText(score_text_buffer, 20, 20, 30, VIOLET);
            (void)snprintf(
                level_text_buffer, TEXT_BUFFER_LENGTH, "Level %02d/%d", state.level, MAX_LEVEL);
            DrawText(level_text_buffer, 20, 60, 20, BLUE);

            DrawFPS(GetScreenWidth() - 100, 20);
        } break;
        case ENDING: {
            (void)snprintf(score_text_buffer, TEXT_BUFFER_LENGTH, "Score %d", state.score);
            DrawText(score_text_buffer, 20, 20, 30, VIOLET);
            (void)snprintf(
                level_text_buffer, TEXT_BUFFER_LENGTH, "Level %02d/%d", state.level, MAX_LEVEL);
            DrawText(level_text_buffer, 20, 60, 20, BLUE);

            const char *game_over_text = "Game over!";
            DrawText(game_over_text,
                     GetScreenWidth() / 2 - MeasureText(game_over_text, 40) / 2,
                     GetScreenHeight() / 2 - 40,
                     40,
                     RED);
            const char *play_again_text = "Press [ENTER] to play again";
            DrawText(play_again_text,
                     GetScreenWidth() / 2 - MeasureText(play_again_text, 20) / 2,
                     GetScreenHeight() / 2 + 20,
                     20,
                     DARKGRAY);
        } break;
        }

        EndDrawing();
    }

    // De-Initialization
    //---------------------------------------------------------------------------------------------
    UnloadSound(fx_spawn);
    UnloadSound(fx_collision);
    CloseAudioDevice();

    CloseWindow();

    return EXIT_SUCCESS;
}
