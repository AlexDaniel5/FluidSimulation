#include "renderer.h"

static SDL_Texture* fluidTex = nullptr;

void renderer_init(SDL_Renderer* renderer) {
    fluidTex = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        GRID_WIDTH, GRID_HEIGHT
    );
    SDL_SetTextureBlendMode(fluidTex, SDL_BLENDMODE_BLEND);
}

static void draw_background(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

static void draw_fluid(SDL_Renderer* renderer, const FluidState& fluid) {
    if (!fluidTex) return;

    void* pixels;
    int pitch;
    SDL_LockTexture(fluidTex, nullptr, &pixels, &pitch);

    Uint32* px = static_cast<Uint32*>(pixels);
    int rowStride = pitch / 4;
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            float d = fluid.dens[IX(x + 1, y + 1)];
            if (d < 0.0f) d = 0.0f;
            if (d > 1.0f) d = 1.0f;
            d = d * d * (3.0f - 2.0f * d);  // smoothstep: sharpens the surface
            Uint8 a = (Uint8)(255.0f * d);
            Uint8 g = (Uint8)(50.0f  * d);
            Uint8 b = (Uint8)(200.0f * d);
            px[y * rowStride + x] = ((Uint32)a << 24) | ((Uint32)g << 8) | b;
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

void renderer_draw(SDL_Renderer* renderer, const FluidState& fluid) {
    draw_background(renderer);
    draw_fluid(renderer, fluid);
    draw_cells(renderer);
    draw_grid_lines(renderer);
}

void renderer_destroy() {
    if (fluidTex) {
        SDL_DestroyTexture(fluidTex);
        fluidTex = nullptr;
    }
}
