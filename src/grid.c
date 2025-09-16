#include "grid.h"
#include <stdio.h>

// Statically initialize the GRID to 3x3
//
// Rendering rules are as follows:
//
// 1. If there are N windows in a row, they will be split equally
// 2. If there are N windows in a column, they will be split equally
static Grid grid = { .windows = {
  {NULL, NULL, NULL},
  {NULL, NULL, NULL},
  {NULL, NULL, NULL},
}};

static void _printBoxGrid() {
  for (int i = 0; i < GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      if (grid.windows[i][j] != NULL) {
        printf("%d ", grid.windows[i][j]->hwnd);
      } else {
        printf("O ");
      }
    }
    printf("\n");
  }
}

int grid_left_count(const Grid *grid) {
  int leftCount = 0;

  for (int i = 0; i < GRID_ROWS; i++) {
    if (grid->windows[i][0] != NULL) {
      leftCount++;
    }
  }

  return leftCount;
}

int grid_right_count(const Grid *grid) {
  int rightCount = 0;

  for (int i = 0; i < GRID_ROWS; i++) {
    if (grid->windows[i][1] != NULL) {
      rightCount++;
    }
  }

  return rightCount;
}

/**
 * Defines a template for a grid layout e.g.
 * ______________________
 * |         |__________|
 * |         |__________|
 * |_________|__________|
 */
const int RIGHT_VERTICAL[GRID_ROWS][GRID_COLS] = {
    {0, 1, 2},
    {0, 1, 3},
    {0, 1, 4},
};

Grid *grid_assign_handles(WindowHandle windowHandles[], const int windowCount) {
  for (int windowIndex = 0; windowIndex < windowCount; windowIndex++) {
    for (int i = 0; i < GRID_ROWS; i++) {
      for (int j = 0; j < GRID_COLS; j++) {
        if (RIGHT_VERTICAL[i][j] == windowIndex + 1) {
          grid.windows[i][j] = &windowHandles[windowIndex];
        }
      }
    }
  }

  _printBoxGrid();
  return &grid;
}

GridCoordinate grid_find_window(const Grid *grid, const WindowHandle *windowHandle) {
  GridCoordinate coordinate = { .x = -1, .y = -1 };

  for (int i = 0; i < GRID_ROWS; i++) {
    for (int j = 0; j < GRID_COLS; j++) {
      if (grid->windows[i][j] != NULL && grid->windows[i][j]->hwnd == windowHandle->hwnd) {
        coordinate.x = i;
        coordinate.y = j;

        return coordinate;
      }
    }
  }

  return coordinate;
}