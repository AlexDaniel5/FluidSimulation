#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <SDL.h>
#include "grid.h"
#include "fluid.h"
#include "renderer.h"
#include "input.h"

// Single definition of the shared grid
int grid[GRID_HEIGHT][GRID_WIDTH] = {0};

static void update_sand() {
    for (int y = GRID_HEIGHT - 2; y >= 0; --y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            if (grid[y][x] != SAND) continue;

            if (grid[y + 1][x] == EMPTY) {
                grid[y + 1][x] = SAND;
                grid[y][x]     = EMPTY;
            } else {
                int dir  = (std::rand() % 2 == 0) ? -1 : 1;
                int newX = x + dir;
                if (newX >= 0 && newX < GRID_WIDTH && grid[y + 1][newX] == EMPTY) {
                    grid[y + 1][newX] = SAND;
                    grid[y][x]        = EMPTY;
                }
            }
        }
    }
}

static const char* mode_name(DrawMode m) {
    switch (m) {
        case MODE_SOLID: return "SOLID";
        case MODE_SAND:  return "SAND";
        case MODE_FLUID: return "FLUID";
        default:         return "?";
    }
}

int main(int argc, char* argv[]) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cout << "Could not initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "FluidSimulation [SOLID]",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WIDTH, HEIGHT,
        SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!window) {
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    FluidState fluid;
    fluid_init(fluid);

    InputState input;
    input_init(input);

    renderer_init(renderer);

    int frameCount = 0;
    bool running = true;
    SDL_Event event;
    DrawMode prevMode = input.mode;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (!input_handle_event(input, event, fluid))
                running = false;
        }

        // Update window title when draw mode changes
        if (input.mode != prevMode) {
            char title[64];
            std::snprintf(title, sizeof(title), "FluidSimulation [%s]", mode_name(input.mode));
            SDL_SetWindowTitle(window, title);
            prevMode = input.mode;
        }

        frameCount++;
        if (frameCount % 6 == 0)
            update_sand();

        fluid_step(fluid, 0.1f);

        renderer_draw(renderer, fluid);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    renderer_destroy();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
