#pragma once
#include "grid.h"

struct FluidState {
    float u[TOTAL];
    float u0[TOTAL];
    float v[TOTAL];
    float v0[TOTAL];
    float dens[TOTAL];
    float dens0[TOTAL];
};

void fluid_init(FluidState& state);
void fluid_reset(FluidState& state);
void fluid_step(FluidState& state, float dt);
void fluid_add_density(FluidState& state, int x, int y, float amount);
void fluid_add_velocity(FluidState& state, int x, int y, float vx, float vy);
