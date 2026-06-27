#pragma once
#include <SDL.h>
#include "grid.h"
#include "fluid.h"

enum DrawMode { MODE_SOLID = 0, MODE_SAND, MODE_FLUID };

// Brush radius in cells (0 = single cell). Adjusted with [ and ].
const int BRUSH_MIN = 0;
const int BRUSH_MAX = 5;

struct InputState {
    bool isDrawing;
    bool showGrid;  // debug grid-line overlay, toggled with G
    DrawMode mode;
    int brushRadius;
    int prevMouseX;
    int prevMouseY;
};

void input_init(InputState& state);
// Returns false when the user requests quit.
bool input_handle_event(InputState& state, const SDL_Event& event, FluidState& fluid);
