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

#define LEVEL_UP_EFFECT_DURATION 2.0F   // seconds
#define COLLISION_EFFECT_DURATION 1.55F // seconds (match collision sound)
#define COLLISION_EFFECT_FREQUENCY 6    // flashes per second
#define SPAWN_EFFECT_DURATION 0.3F      // seconds

#define TEXT_BUFFER_LENGTH 1024

#define DEBUG_INVINCIBLE false

typedef enum {
    TITLE,
    GAMEPLAY,
    PAUSED,
    COLLISION,
    ENDING,
} GameScreen;

typedef struct {
    Vector2 position;
} Player;

typedef struct {
    Vector2 position;
    bool active;
    bool spawn_effect_active;
    float spawn_effect_timer;
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
    bool level_up_effect_active;
    float level_up_effect_timer;
    bool collision_effect_active;
    float collision_effect_timer;
} GameState;

void block_spawn(Block *block, Vector2 position) {
    block->position = position;
    block->active = true;
    block->spawn_effect_active = true;
    block->spawn_effect_timer = SPAWN_EFFECT_DURATION;
}

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
    block_spawn(&state->blocks[0],
                (Vector2){
                    .x = (float)GetScreenWidth() / 2 - (float)BLOCK_SIZE / 2,
                    .y = 10,
                });

    state->time_since_last_spawn = 0.0F;
    state->time_since_last_level = 0.0F;

    state->score = 0;
    state->level = 1;
    state->block_speed = BLOCK_SPEED_INITIAL;
    state->spawn_interval = SPAWN_INTERVAL_INITIAL;

    state->level_up_effect_active = false;
    state->level_up_effect_timer = 0.0F;
    state->collision_effect_active = false;
    state->collision_effect_timer = 0.0F;
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

            // Pause gameplay
            if (IsKeyPressed(KEY_P)) {
                screen = PAUSED;
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

                if (block->spawn_effect_active) {
                    block->spawn_effect_timer -= GetFrameTime();
                    if (block->spawn_effect_timer <= 0) {
                        block->spawn_effect_active = false;
                    }
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
                        PlaySound(fx_collision);
                        state.collision_effect_active = true;
                        state.collision_effect_timer = COLLISION_EFFECT_DURATION;

                        screen = COLLISION;
                    }
                }
            }

            // Change level
            state.time_since_last_level += GetFrameTime();
            if (state.time_since_last_level > LEVEL_CHANGE_INTERVAL && state.level < MAX_LEVEL) {
                state.time_since_last_level = 0.0F;
                state.level += 1;

                state.block_speed = state.block_speed * (1.0F + BLOCK_SPEED_LEVEL_INCREASE);
                state.spawn_interval =
                    state.spawn_interval * (1.0F - SPAWN_INTERVAL_LEVEL_DECREASE);

                state.level_up_effect_active = true;
                state.level_up_effect_timer = LEVEL_UP_EFFECT_DURATION;
            }
            if (state.level_up_effect_active) {
                state.level_up_effect_timer -= GetFrameTime();

                if (state.level_up_effect_timer <= 0) {
                    state.level_up_effect_active = false;
                }
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
                    Vector2 position = {
                        .x = (float)GetRandomValue(0, GetScreenWidth() - BLOCK_SIZE),
                        .y = 10,
                    };
                    block_spawn(block, position);

                    PlaySound(fx_spawn);
                }
            }
        } break;
        case PAUSED: {
            if (IsKeyPressed(KEY_P)) {
                screen = GAMEPLAY;
            }
        } break;
        case COLLISION: {
            state.collision_effect_timer -= GetFrameTime();
            if (state.collision_effect_timer < 0) {
                screen = ENDING;
            }
        } break;
        case ENDING: {
            if (IsKeyPressed(KEY_ENTER)) {
                screen = GAMEPLAY;
                game_state_reset(&state);
                PlaySound(fx_spawn);
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
            int title_font_size = 40;
            DrawText(title_text,
                     GetScreenWidth() / 2 - MeasureText(title_text, title_font_size) / 2,
                     GetScreenHeight() / 2 - title_font_size,
                     title_font_size,
                     DARKGREEN);
            const char *start_text = "Press [ENTER] to start";
            int start_font_size = 20;
            DrawText(start_text,
                     GetScreenWidth() / 2 - MeasureText(start_text, start_font_size) / 2,
                     GetScreenHeight() / 2 + 20,
                     start_font_size,
                     DARKGRAY);

        } break;
        case GAMEPLAY:
        case PAUSED:
        case COLLISION: {
            // Player
            Player *player = &state.player;
            Color player_color = DARKBLUE;
            if (state.collision_effect_active) {
                float elapsed_time = COLLISION_EFFECT_DURATION - state.collision_effect_timer;
                int flash_state = (int)(elapsed_time * COLLISION_EFFECT_FREQUENCY) % 2; // 0 or 1
                if (flash_state == 0) {
                    player_color = RED;
                }
            }
            DrawRectangle((int)player->position.x,
                          (int)player->position.y,
                          PLAYER_WIDTH,
                          PLAYER_HEIGHT,
                          player_color);

            // Blocks
            for (size_t i = 0; i < MAX_BLOCKS; i++) {
                Block *block = &state.blocks[i];
                if (!block->active) {
                    continue;
                }

                float x = block->position.x;
                float y = block->position.y;
                float size = BLOCK_SIZE;
                if (block->spawn_effect_active) {
                    float scale = 1 - block->spawn_effect_timer / SPAWN_EFFECT_DURATION;
                    size = BLOCK_SIZE * scale;
                    x += (BLOCK_SIZE - size) / 2;
                    y += (BLOCK_SIZE - size) / 2;
                }

                DrawRectangle((int)x, (int)y, (int)size, (int)size, MAROON);
            }

            // Score
            (void)snprintf(score_text_buffer, TEXT_BUFFER_LENGTH, "Score %d", state.score);
            DrawText(score_text_buffer, 20, 20, 30, VIOLET);

            // Level indicator
            (void)snprintf(
                level_text_buffer, TEXT_BUFFER_LENGTH, "Level %02d/%d", state.level, MAX_LEVEL);
            int level_font_size = 20;
            int level_text_x = 20;
            int level_text_y = 60;
            Color level_text_color = BLUE;
            if (state.level_up_effect_active) {
                if (state.level_up_effect_timer > LEVEL_UP_EFFECT_DURATION / 2) {
                    level_text_color = WHITE;
                } else {
                    level_text_color = BLUE;
                }
            }
            if (state.level_up_effect_active) {
                int level_text_width = MeasureText(level_text_buffer, level_font_size);
                int padding = 10;
                DrawRectangle(level_text_x - padding,
                              level_text_y - padding,
                              level_text_width + 2 * padding,
                              level_font_size + 2 * padding,
                              Fade(BLUE, state.level_up_effect_timer / LEVEL_UP_EFFECT_DURATION));
            }
            DrawText(level_text_buffer, level_text_x, 60, level_font_size, level_text_color);

            // Level up message
            if (state.level_up_effect_active) {
                const char *level_up_text = "LEVEL UP!";
                int level_up_font_size = 40;
                DrawText(level_up_text,
                         GetScreenWidth() / 2 - MeasureText(level_up_text, level_up_font_size) / 2,
                         GetScreenHeight() / 2 - level_up_font_size,
                         level_up_font_size,
                         Fade(BLUE, state.level_up_effect_timer / LEVEL_UP_EFFECT_DURATION));
            }

            // Paused message
            if (screen == PAUSED) {
                const char *paused_text = "PAUSED";
                int paused_font_size = 40;
                int paused_text_width = MeasureText(paused_text, paused_font_size);
                int paused_text_x = GetScreenWidth() / 2 - paused_text_width / 2;
                int paused_text_y = GetScreenHeight() / 2 - paused_font_size;
                int padding = 10;
                DrawRectangle(paused_text_x - padding,
                              paused_text_y - padding,
                              paused_text_width + 2 * padding,
                              paused_font_size + 2 * padding,
                              RAYWHITE);
                DrawText(paused_text, paused_text_x, paused_text_y, paused_font_size, GRAY);
            }

            // FPS
            DrawFPS(GetScreenWidth() - 100, 20);
        } break;
        case ENDING: {
            (void)snprintf(score_text_buffer, TEXT_BUFFER_LENGTH, "Score %d", state.score);
            DrawText(score_text_buffer, 20, 20, 30, VIOLET);
            (void)snprintf(
                level_text_buffer, TEXT_BUFFER_LENGTH, "Level %02d/%d", state.level, MAX_LEVEL);
            DrawText(level_text_buffer, 20, 60, 20, BLUE);

            const char *game_over_text = "Game over!";
            int game_over_font_size = 40;
            DrawText(game_over_text,
                     GetScreenWidth() / 2 - MeasureText(game_over_text, game_over_font_size) / 2,
                     GetScreenHeight() / 2 - game_over_font_size,
                     game_over_font_size,
                     RED);
            const char *play_again_text = "Press [ENTER] to play again";
            int play_again_font_size = 20;
            DrawText(play_again_text,
                     GetScreenWidth() / 2 - MeasureText(play_again_text, play_again_font_size) / 2,
                     GetScreenHeight() / 2 + 20,
                     play_again_font_size,
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
