#ifndef GRID_H
#define GRID_H
#include "window_manager.h"

// Define the grid layout
#define GRID_ROWS 3
#define GRID_COLS 3

typedef struct {
    WindowHandle* windows[GRID_ROWS][GRID_COLS];
} Grid;

typedef struct {
    int x;
    int y;
} GridCoordinate;

Grid *grid_assign_handles(WindowHandle windowHandles[], const int windowCount);
int grid_left_count(const Grid *grid);
int grid_right_count(const Grid *grid);
GridCoordinate grid_find_window(const Grid *grid, const WindowHandle *windowHandle);

#endif //GRID_H
