#include "pool.h"
#include "raylib.h"
#include "raymath.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 450

#define TEXT_BUFFER_LENGTH 1024

// Note: Can calculate approximate particle count when continuously holding
// mouse button down using particle lifetime (L) particle emission interval (E)
// N(T) = T/E                    for 0 ≤ T < L
// N(T) = T/E − (T − L)/E = L/E  for T ≥ L
// (where T is seconds since emission began)
#define MAX_PARTICLES 500
// #define MAX_PARTICLES 10 // for debugging

#define GRAVITY_ACCELERATION 50.0F // pixels per second^2
// i have no idea what to put here haha

#define PARTICLE_RADIUS 10.0F           // pixels
#define PARTICLE_INITIAL_SPEED 150.0F   // pixels per second
#define PARTICLE_LIFETIME 5.0F          // seconds
#define PARTICLE_EMISSION_INTERVAL 0.1F // seconds

typedef struct {
    Vector2 velocity;
    Vector2 position;
    float age; // seconds, count up
} Particle;

typedef struct {
    Pool *pool;
    Particle *particles[MAX_PARTICLES];
    size_t particle_count;
    float particle_emission_timer; // count down, may cross zero
} SimulationState;

static void particle_init(Particle *particle) {
    // Initial velocity: random vector direction in "top half-circle"
    float u = (float)GetRandomValue(0, 100) / 100;
    float angle = PI * (1 + u);
    particle->velocity = Vector2Scale(
        (Vector2){
            .x = cosf(angle),
            .y = sinf(angle),
        },
        PARTICLE_INITIAL_SPEED);
    // Initial position: mouse click location
    particle->position = (Vector2){
        .x = (float)GetMouseX(),
        .y = (float)GetMouseY(),
    };
    particle->age = 0;
}

static void simulation_state_reset(SimulationState *state) {
    pool_reset(state->pool);
    state->particle_count = 0;
    state->particle_emission_timer = 0;
}

static bool particles_spawn(SimulationState *state) {
    // Handle capacity exhaustion gracefully: don't render any more particles
    if (state->particle_count >= MAX_PARTICLES) {
        return false;
    }

    size_t i = state->particle_count;

    Particle *particle = pool_alloc(state->pool);
    assert(particle != NULL && "expected particle pool to have free slots");

    particle_init(particle);
    state->particles[i] = particle;
    state->particle_count++;

    return true;
}

static void particles_update_emission(SimulationState *state) {
    if (state->particle_emission_timer > 0) {
        float dt = GetFrameTime();
        state->particle_emission_timer -= dt;
    }
}

static bool particles_is_emission_due(SimulationState *state) {
    return (state->particle_emission_timer <= 0);
}

static void particles_move(SimulationState *state) {
    float dt = GetFrameTime();

    const Vector2 particle_acceleration = (Vector2){
        .x = 0,
        .y = GRAVITY_ACCELERATION,
    };

    for (size_t i = 0; i < state->particle_count; i++) {
        Particle *particle = state->particles[i];

        // Move
        // Use semi-implicit Euler (update velocity before position)
        particle->velocity =
            Vector2Add(particle->velocity, Vector2Scale(particle_acceleration, dt));
        particle->position = Vector2Add(particle->position, Vector2Scale(particle->velocity, dt));
    }
}

static void particles_age(SimulationState *state) {
    float dt = GetFrameTime();

    size_t i = 0;
    // age, expire, and compact array in single loop
    while (i < state->particle_count) {
        Particle *particle = state->particles[i];

        particle->age += dt;

        // Expiry
        if (particle->age >= PARTICLE_LIFETIME) {
            pool_free(state->pool, particle);

            // Compact array using swap removal:
            // till expired entry with last entry (except if expired is already last)
            // and decrement count
            if (i != state->particle_count - 1) {
                Particle *last = state->particles[state->particle_count - 1];
                state->particles[i] = last;
            }
            state->particle_count--;
            // Process same index again since we swapped it (i.e. don't increment it)
        } else {
            i++;
        }
    }
}

static void particles_draw(SimulationState *state) {
    for (size_t i = 0; i < state->particle_count; i++) {
        Particle *particle = state->particles[i];

        DrawCircleV(particle->position, PARTICLE_RADIUS, DARKGREEN);
    }
}

static void capacity_draw(SimulationState *state) {
    char text_buffer[TEXT_BUFFER_LENGTH] = {0};
    (void)snprintf(
        text_buffer, TEXT_BUFFER_LENGTH, "Particles %zu/%d", state->particle_count, MAX_PARTICLES);
    int font_size = 20;
    int text_width = MeasureText(text_buffer, font_size);
    DrawText(text_buffer,
             GetScreenWidth() - text_width - 20,
             GetScreenHeight() - font_size - 20,
             font_size,
             VIOLET);
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

    // For debugging
    SetRandomSeed(123456);

    while (!WindowShouldClose()) {
        // Update
        //-----------------------------------------------------------------------------------------
        particles_age(&state);
        particles_move(&state);
        particles_update_emission(&state);

        // Spawn after move so first appears right under the mouse
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            bool particle_spawned = particles_spawn(&state);
            if (particle_spawned) {
                // Reset timer to full interval in the case of a single click
                // (instead of adding interval like emission path)
                state.particle_emission_timer = PARTICLE_EMISSION_INTERVAL;
            }
        } else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && particles_is_emission_due(&state)) {
            bool particle_spawned = particles_spawn(&state);
            if (particle_spawned) {
                // Timer may have some "leftover" and be negative,
                // add interval to timer instead of resetting it
                // so continuous spawning still feels smooth
                state.particle_emission_timer += PARTICLE_EMISSION_INTERVAL;
            }
        }

        if (IsKeyPressed(KEY_R)) {
            simulation_state_reset(&state);
        }
        // Draw
        //-----------------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        particles_draw(&state);
        capacity_draw(&state);
        DrawFPS(GetScreenWidth() - 100, 20);

        EndDrawing();
    }

    // Cleanup
    //---------------------------------------------------------------------------------------------
    pool_destroy(state.pool);

    CloseWindow();

    return EXIT_SUCCESS;
}
