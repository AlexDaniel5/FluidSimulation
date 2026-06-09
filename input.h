#pragma once
#include <SDL.h>
#include "grid.h"
#include "fluid.h"

enum DrawMode { MODE_SOLID = 0, MODE_SAND, MODE_FLUID };

struct InputState {
    bool isDrawing;
    DrawMode mode;
    int prevMouseX;
    int prevMouseY;
};

void input_init(InputState& state);
// Returns false when the user requests quit.
bool input_handle_event(InputState& state, const SDL_Event& event, FluidState& fluid);
