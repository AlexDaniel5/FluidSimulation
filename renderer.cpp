#include "renderer.h"
#include <vector>
#include <cmath>
#include <algorithm>

// Water is rendered from the particles: each one splats a radial blob into a
// half-resolution field, which is thresholded into a metaball-style surface.
static const int   RW = WIDTH / 2;
static const int   RH = HEIGHT / 2;
static const float PSCALE = CELL_SIZE * 0.5f;  // half-res pixels per cell
static const float BLOB_R = 0.9f * PSCALE;     // splat radius, in half-res px

static SDL_Texture* fluidTex = nullptr;
static std::vector<float> blobF;   // blob coverage
static std::vector<float> speedF;  // blob-weighted particle speed

void renderer_init(SDL_Renderer* renderer) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");  // smooth upscale
    fluidTex = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        RW, RH
    );
    SDL_SetTextureBlendMode(fluidTex, SDL_BLENDMODE_BLEND);
    blobF.resize(RW * RH);
    speedF.resize(RW * RH);
}

static void draw_background(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

static float smoothstep(float e0, float e1, float x) {
    float t = (x - e0) / (e1 - e0);
    t = std::max(0.0f, std::min(t, 1.0f));
    return t * t * (3.0f - 2.0f * t);
}

static Uint8 lerp8(int a, int b, float t) {
    return (Uint8)(a + (b - a) * t);
}

static void splat_particles(const FluidState& fluid) {
    std::fill(blobF.begin(), blobF.end(), 0.0f);
    std::fill(speedF.begin(), speedF.end(), 0.0f);

    const float invR2 = 1.0f / (BLOB_R * BLOB_R);
    for (const Particle& pt : fluid.particles) {
        float fx = pt.x * PSCALE;
        float fy = pt.y * PSCALE;
        float speed = std::sqrt(pt.vx * pt.vx + pt.vy * pt.vy);

        int x0 = std::max(0, (int)(fx - BLOB_R));
        int x1 = std::min(RW - 1, (int)(fx + BLOB_R));
        int y0 = std::max(0, (int)(fy - BLOB_R));
        int y1 = std::min(RH - 1, (int)(fy + BLOB_R));
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                float dx = x + 0.5f - fx;
                float dy = y + 0.5f - fy;
                float d2 = (dx * dx + dy * dy) * invR2;
                if (d2 >= 1.0f) continue;
                float w = 1.0f - d2;
                w *= w;
                blobF[y * RW + x] += w;
                speedF[y * RW + x] += w * speed;
            }
        }
    }
}

static void draw_fluid(SDL_Renderer* renderer, const FluidState& fluid) {
    if (!fluidTex) return;

    splat_particles(fluid);

    void* pixels;
    int pitch;
    SDL_LockTexture(fluidTex, nullptr, &pixels, &pitch);
    Uint32* px = static_cast<Uint32*>(pixels);
    int rowStride = pitch / 4;

    // Walk each column top-down so a running counter gives depth below the
    // local surface (it resets across gaps, so droplets shade independently).
    for (int x = 0; x < RW; ++x) {
        int wetRun = 0;
        for (int y = 0; y < RH; ++y) {
            float b = blobF[y * RW + x];
            float a = smoothstep(0.35f, 0.7f, b);
            if (a < 0.02f) {
                wetRun = 0;
                px[y * rowStride + x] = 0;
                continue;
            }
            wetRun++;

            float depth = wetRun / PSCALE;                       // in cells
            float deep = std::min(depth / 15.0f, 1.0f);          // darken
            float speed = speedF[y * RW + x] / b;
            float foam = smoothstep(8.0f, 30.0f, speed) * 0.8f;  // lighten

            Uint8 r = lerp8(lerp8(40, 8, deep),   210, foam);
            Uint8 g = lerp8(lerp8(110, 30, deep), 230, foam);
            Uint8 bl = lerp8(lerp8(220, 90, deep), 250, foam);
            Uint8 al = (Uint8)(235.0f * a);
            px[y * rowStride + x] =
                ((Uint32)al << 24) | ((Uint32)r << 16) | ((Uint32)g << 8) | bl;
        }
    }

    SDL_UnlockTexture(fluidTex);
    SDL_Rect dst = {0, 0, WIDTH, HEIGHT};
    SDL_RenderCopy(renderer, fluidTex, nullptr, &dst);
}

static void draw_cell(SDL_Renderer* renderer, int x, int y, int type) {
    SDL_Color color = (type == SOLID)
        ? SDL_Color{200, 200, 200, 255}
        : SDL_Color{220, 180, 100, 255};  // sand = warm tan
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect cell = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
    SDL_RenderFillRect(renderer, &cell);
}

static void draw_cells(SDL_Renderer* renderer) {
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            if (grid[y][x] != EMPTY)
                draw_cell(renderer, x, y, grid[y][x]);
        }
    }
}

static void draw_grid_lines(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    for (int x = 0; x < WIDTH; x += CELL_SIZE)
        SDL_RenderDrawLine(renderer, x, 0, x, HEIGHT);
    for (int y = 0; y < HEIGHT; y += CELL_SIZE)
        SDL_RenderDrawLine(renderer, 0, y, WIDTH, y);
}

void renderer_draw(SDL_Renderer* renderer, const FluidState& fluid, bool showGrid) {
    draw_background(renderer);
    draw_fluid(renderer, fluid);
    draw_cells(renderer);
    if (showGrid)
        draw_grid_lines(renderer);
}

void renderer_destroy() {
    if (fluidTex) {
        SDL_DestroyTexture(fluidTex);
        fluidTex = nullptr;
    }
}
