#include "input.h"
#include <cstring>

void input_init(InputState& state) {
    state.isDrawing  = false;
    state.mode       = MODE_SOLID;
    state.prevMouseX = 0;
    state.prevMouseY = 0;
}

bool input_handle_event(InputState& state, const SDL_Event& event, FluidState& fluid) {
    if (event.type == SDL_QUIT) return false;

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        state.isDrawing  = true;
        state.prevMouseX = event.button.x;
        state.prevMouseY = event.button.y;
    } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        state.isDrawing = false;
    } else if (event.type == SDL_MOUSEMOTION && state.isDrawing) {
        int cx = event.motion.x / CELL_SIZE;
        int cy = event.motion.y / CELL_SIZE;
        if (cx >= 0 && cx < GRID_WIDTH && cy >= 0 && cy < GRID_HEIGHT) {
            if (state.mode == MODE_SOLID) {
                grid[cy][cx] = SOLID;
            } else if (state.mode == MODE_SAND) {
                grid[cy][cx] = SAND;
            } else {
                float dx = (float)(event.motion.x - state.prevMouseX);
                float dy = (float)(event.motion.y - state.prevMouseY);
                fluid_add_density(fluid, cx, cy, 0.5f);
                // Particle velocities are in cells/second; turn the per-event
                // mouse delta into a fling impulse.
                fluid_add_velocity(fluid, cx, cy, dx / CELL_SIZE * 4.0f, dy / CELL_SIZE * 4.0f);
            }
        }
        state.prevMouseX = event.motion.x;
        state.prevMouseY = event.motion.y;
    }

    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_SPACE:
                state.mode = static_cast<DrawMode>((state.mode + 1) % 3);
                break;
            case SDLK_r:
                fluid_reset(fluid);
                break;
            case SDLK_c:
                fluid_reset(fluid);
                memset(grid, 0, sizeof(int) * GRID_HEIGHT * GRID_WIDTH);
                break;
        }
    }

    return true;
}
