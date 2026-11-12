#include "pool.h"
#include "raylib.h"

#include <assert.h>
#include <raymath.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 450

// #define MAX_PARTICLES 500
#define MAX_PARTICLES 10 // for debugging

#define GRAVITY_ACCELERATION 50.0F // pixels per second^2
// i have no idea what to put here haha

#define PARTICLE_RADIUS 10.0F         // pixels
#define PARTICLE_INITIAL_SPEED 150.0F // pixels per second

typedef struct {
    Vector2 velocity;
    Vector2 position;
} Particle;

typedef struct {
    Pool *pool;
    Particle *particles[MAX_PARTICLES];
    size_t particle_count;
} SimulationState;

static void particle_reset(Particle *particle) {
    // initial velocity (direction will be randomized later)
    particle->velocity = Vector2Scale(Vector2Normalize((Vector2){
                                          .x = 1,
                                          .y = -1,
                                      }),
                                      PARTICLE_INITIAL_SPEED);
    // initial position: mouse click location
    particle->position = (Vector2){
        .x = (float)GetMouseX(),
        .y = (float)GetMouseY(),
    };
}

static void simulation_state_reset(SimulationState *state) {
    pool_reset(state->pool);
    state->particle_count = 0;
}

static void particles_spawn(SimulationState *state) {
    // Handle capacity exhaustion gracefully: don't render any more particles
    if (state->particle_count >= MAX_PARTICLES) {
        return;
    }

    size_t i = state->particle_count;

    Particle *particle = pool_alloc(state->pool);
    assert(particle != NULL && "expected particle pool to have free slots");

    particle_reset(particle);
    state->particles[i] = particle;
    state->particle_count++;
}

static void particles_move(SimulationState *state) {
    float dt = GetFrameTime();

    const Vector2 particle_acceleration = (Vector2){
        .x = 0,
        .y = GRAVITY_ACCELERATION,
    };

    for (size_t i = 0; i < state->particle_count; i++) {
        Particle *particle = state->particles[i];

        // Semi-implicit Euler (update velocity before position)
        particle->velocity =
            Vector2Add(particle->velocity, Vector2Scale(particle_acceleration, dt));
        particle->position = Vector2Add(particle->position, Vector2Scale(particle->velocity, dt));
    }
}

static void particles_draw(SimulationState *state) {
    for (size_t i = 0; i < state->particle_count; i++) {
        Particle *particle = state->particles[i];

        DrawCircleV(particle->position, PARTICLE_RADIUS, DARKGREEN);
    }
}

int main(void) {
    // Initialization
    //---------------------------------------------------------------------------------------------
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Particle Simulation");

    SetTargetFPS(60);

    // State
    SimulationState state = {0};
    Pool *pool = pool_create(Particle, MAX_PARTICLES);
    if (pool == NULL) {
        fprintf(stderr, "[ERROR] Could not allocate memory for particles\n");
        CloseWindow();
        return EXIT_FAILURE;
    }
    state.pool = pool;

    while (!WindowShouldClose()) {
        // Update
        //-----------------------------------------------------------------------------------------
        particles_move(&state);

        // Spawn after move so first appears right under the mouse
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            particles_spawn(&state);
        }

        if (IsKeyPressed(KEY_R)) {
            simulation_state_reset(&state);
        }
        // Draw
        //-----------------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        particles_draw(&state);

        DrawFPS(GetScreenWidth() - 100, 20);

        EndDrawing();
    }

    // Cleanup
    //---------------------------------------------------------------------------------------------
    pool_destroy(state.pool);

    CloseWindow();

    return EXIT_SUCCESS;
}
