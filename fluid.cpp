#include "fluid.h"
#include <cstring>
#include <cmath>

// ---- helpers ----

static void add_source(int n, float* x, float* s, float dt) {
    for (int i = 0; i < n; ++i)
        x[i] += dt * s[i];
}

static void set_boundary(int b, float* x) {
    // Edges
    for (int i = 1; i <= GRID_HEIGHT; ++i) {
        x[IX(0, i)]           = (b == 1) ? -x[IX(1, i)]           : x[IX(1, i)];
        x[IX(GRID_WIDTH+1, i)] = (b == 1) ? -x[IX(GRID_WIDTH, i)] : x[IX(GRID_WIDTH, i)];
    }
    for (int j = 1; j <= GRID_WIDTH; ++j) {
        x[IX(j, 0)]            = (b == 2) ? -x[IX(j, 1)]           : x[IX(j, 1)];
        x[IX(j, GRID_HEIGHT+1)] = (b == 2) ? -x[IX(j, GRID_HEIGHT)] : x[IX(j, GRID_HEIGHT)];
    }
    // Corners
    x[IX(0, 0)]                         = 0.5f * (x[IX(1, 0)]           + x[IX(0, 1)]);
    x[IX(GRID_WIDTH+1, 0)]              = 0.5f * (x[IX(GRID_WIDTH, 0)]  + x[IX(GRID_WIDTH+1, 1)]);
    x[IX(0, GRID_HEIGHT+1)]             = 0.5f * (x[IX(1, GRID_HEIGHT+1)] + x[IX(0, GRID_HEIGHT)]);
    x[IX(GRID_WIDTH+1, GRID_HEIGHT+1)]  = 0.5f * (x[IX(GRID_WIDTH, GRID_HEIGHT+1)] + x[IX(GRID_WIDTH+1, GRID_HEIGHT)]);

    // Solid obstacle cells
    for (int j = 1; j <= GRID_HEIGHT; ++j) {
        for (int i = 1; i <= GRID_WIDTH; ++i) {
            if (grid[j-1][i-1] == SOLID) {
                x[IX(i, j)] = 0.0f;
            }
        }
    }
}

static void diffuse(int b, float* x, float* x0, float diff, float dt, int iter) {
    float a = dt * diff * GRID_WIDTH * GRID_HEIGHT;
    float inv = 1.0f / (1.0f + 4.0f * a);
    for (int k = 0; k < iter; ++k) {
        for (int j = 1; j <= GRID_HEIGHT; ++j) {
            for (int i = 1; i <= GRID_WIDTH; ++i) {
                x[IX(i, j)] = (x0[IX(i, j)] + a * (
                    x[IX(i-1, j)] + x[IX(i+1, j)] +
                    x[IX(i, j-1)] + x[IX(i, j+1)]
                )) * inv;
            }
        }
        set_boundary(b, x);
    }
}

static void advect(int b, float* d, float* d0, float* u, float* v, float dt) {
    float dtx = dt * GRID_WIDTH;
    float dty = dt * GRID_HEIGHT;

    for (int j = 1; j <= GRID_HEIGHT; ++j) {
        for (int i = 1; i <= GRID_WIDTH; ++i) {
            float x = i - dtx * u[IX(i, j)];
            float y = j - dty * v[IX(i, j)];

            if (x < 0.5f) x = 0.5f;
            if (x > GRID_WIDTH + 0.5f) x = GRID_WIDTH + 0.5f;
            int i0 = (int)x; int i1 = i0 + 1;

            if (y < 0.5f) y = 0.5f;
            if (y > GRID_HEIGHT + 0.5f) y = GRID_HEIGHT + 0.5f;
            int j0 = (int)y; int j1 = j0 + 1;

            float s1 = x - i0; float s0 = 1.0f - s1;
            float t1 = y - j0; float t0 = 1.0f - t1;

            d[IX(i, j)] =
                s0 * (t0 * d0[IX(i0, j0)] + t1 * d0[IX(i0, j1)]) +
                s1 * (t0 * d0[IX(i1, j0)] + t1 * d0[IX(i1, j1)]);
        }
    }
    set_boundary(b, d);
}

static void project(float* u, float* v, float* p, float* div, int iter) {
    float hx = 1.0f / GRID_WIDTH;
    float hy = 1.0f / GRID_HEIGHT;

    for (int j = 1; j <= GRID_HEIGHT; ++j) {
        for (int i = 1; i <= GRID_WIDTH; ++i) {
            div[IX(i, j)] = -0.5f * (
                hx * (u[IX(i+1, j)] - u[IX(i-1, j)]) +
                hy * (v[IX(i, j+1)] - v[IX(i, j-1)])
            );
            p[IX(i, j)] = 0.0f;
        }
    }
    set_boundary(0, div);
    set_boundary(0, p);

    for (int k = 0; k < iter; ++k) {
        for (int j = 1; j <= GRID_HEIGHT; ++j) {
            for (int i = 1; i <= GRID_WIDTH; ++i) {
                p[IX(i, j)] = (div[IX(i, j)] + p[IX(i-1, j)] + p[IX(i+1, j)] + p[IX(i, j-1)] + p[IX(i, j+1)]) * 0.25f;
            }
        }
        set_boundary(0, p);
    }

    for (int j = 1; j <= GRID_HEIGHT; ++j) {
        for (int i = 1; i <= GRID_WIDTH; ++i) {
            u[IX(i, j)] -= 0.5f * GRID_WIDTH  * (p[IX(i+1, j)] - p[IX(i-1, j)]);
            v[IX(i, j)] -= 0.5f * GRID_HEIGHT * (p[IX(i, j+1)] - p[IX(i, j-1)]);
        }
    }
    set_boundary(1, u);
    set_boundary(2, v);
}

// ---- public API ----

void fluid_init(FluidState& s) {
    memset(&s, 0, sizeof(FluidState));
}

void fluid_reset(FluidState& s) {
    memset(&s, 0, sizeof(FluidState));
}

void fluid_add_density(FluidState& s, int x, int y, float amount) {
    s.dens0[IX(x + 1, y + 1)] += amount;
}

void fluid_add_velocity(FluidState& s, int x, int y, float vx, float vy) {
    s.u0[IX(x + 1, y + 1)] += vx;
    s.v0[IX(x + 1, y + 1)] += vy;
}

void fluid_step(FluidState& s, float dt) {
    const float visc = 0.0f;
    const float diff = 0.0f;
    const int   diffIter = 4;
    const int   projIter = 15;

    // Gravity: pull only where fluid exists (buoyancy coupling)
    const float gravity = 0.02f;
    for (int j = 1; j <= GRID_HEIGHT; ++j)
        for (int i = 1; i <= GRID_WIDTH; ++i)
            s.v0[IX(i, j)] += gravity * s.dens[IX(i, j)];

    // Velocity step
    add_source(TOTAL, s.u, s.u0, dt);
    add_source(TOTAL, s.v, s.v0, dt);
    memset(s.u0, 0, sizeof(s.u0));
    memset(s.v0, 0, sizeof(s.v0));

    // Swap u <-> u0 for diffuse (diffuse from u into u0, then swap back)
    memcpy(s.u0, s.u, sizeof(s.u));
    diffuse(1, s.u, s.u0, visc, dt, diffIter);
    memcpy(s.v0, s.v, sizeof(s.v));
    diffuse(2, s.v, s.v0, visc, dt, diffIter);

    project(s.u, s.v, s.u0, s.v0, projIter);

    // Advect velocity
    float tmpU[TOTAL], tmpV[TOTAL];
    memcpy(tmpU, s.u, sizeof(s.u));
    memcpy(tmpV, s.v, sizeof(s.v));
    advect(1, s.u, tmpU, tmpU, tmpV, dt);
    advect(2, s.v, tmpV, tmpU, tmpV, dt);
    project(s.u, s.v, s.u0, s.v0, projIter);

    // Velocity damping to prevent unbounded growth
    for (int i = 0; i < TOTAL; ++i) {
        s.u[i] *= 0.999f;
        s.v[i] *= 0.999f;
    }

    // Density step
    add_source(TOTAL, s.dens, s.dens0, dt);
    memset(s.dens0, 0, sizeof(s.dens0));

    memcpy(s.dens0, s.dens, sizeof(s.dens));
    diffuse(0, s.dens, s.dens0, diff, dt, diffIter);

    memcpy(s.dens0, s.dens, sizeof(s.dens));
    advect(0, s.dens, s.dens0, s.u, s.v, dt);

    // Clamp density — no decay (liquid conserves volume)
    for (int i = 0; i < TOTAL; ++i)
        if (s.dens[i] < 0.0f) s.dens[i] = 0.0f;
}
