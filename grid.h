#pragma once

const int WIDTH = 800, HEIGHT = 600;
const int CELL_SIZE = 10;
const int GRID_WIDTH = WIDTH / CELL_SIZE;
const int GRID_HEIGHT = HEIGHT / CELL_SIZE;
const int PADDED_W = GRID_WIDTH + 2;
const int PADDED_H = GRID_HEIGHT + 2;
const int TOTAL = PADDED_W * PADDED_H;

#define IX(x, y) ((x) + PADDED_W * (y))

enum CellType { EMPTY = 0, SOLID = 1, SAND = 2 };

extern int grid[GRID_HEIGHT][GRID_WIDTH];
