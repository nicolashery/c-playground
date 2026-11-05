#include "raylib.h"

#include <raymath.h>
#include <stdlib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 450

#define GRAVITY_ACCELERATION 50.0F // pixels per second^2
// i have no idea what to put here haha

#define PARTICLE_RADIUS 10.0F         // pixels
#define PARTICLE_INITIAL_SPEED 150.0F // pixels per second

typedef struct {
    Vector2 velocity;
    Vector2 position;
} Particle;

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

int main(void) {
    // Initialization
    //---------------------------------------------------------------------------------------------
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Particle Simulation");

    SetTargetFPS(60);

    const Vector2 particle_acceleration = (Vector2){
        .x = 0,
        .y = GRAVITY_ACCELERATION,
    };

    // Simulation state
    // single particle for now
    Particle particle = {0};
    particle_reset(&particle);

    while (!WindowShouldClose()) {
        // Update
        //-----------------------------------------------------------------------------------------
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            particle_reset(&particle);
        } else {
            float dt = GetFrameTime();

            // Semi-implicit Euler (update velocity before position)
            particle.velocity =
                Vector2Add(particle.velocity, Vector2Scale(particle_acceleration, dt));
            particle.position = Vector2Add(particle.position, Vector2Scale(particle.velocity, dt));
        }
        // Draw
        //-----------------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawCircleV(particle.position, PARTICLE_RADIUS, DARKGREEN);

        EndDrawing();
    }

    CloseWindow();

    return EXIT_SUCCESS;
}
