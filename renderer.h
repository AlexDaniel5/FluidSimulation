#pragma once
#include <SDL.h>
#include "grid.h"
#include "fluid.h"

void renderer_init(SDL_Renderer* renderer);
void renderer_draw(SDL_Renderer* renderer, const FluidState& fluid);
void renderer_destroy();
