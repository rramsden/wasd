#include "window_manager.h"
#include <windows.h>
#include <stdio.h>

// Function to move the currently active window to the left half of the screen
void moveWindowLeft() {
    // Get the handle of the currently active window
    const HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        RECT rect;
        // Get the current position and size of the window
        GetWindowRect(hwnd, &rect);

        // Get the width of the screen
        const int screenWidth = GetSystemMetrics(SM_CXSCREEN);

        // Calculate the new width for the window (half of the screen width)
        const int newWidth = screenWidth / 2;

        // Calculate the new left position for the window
        int newLeft = rect.left - newWidth;

        // Ensure the new left position is not off the screen
        if (newLeft < 0) {
            newLeft = 0;
        }

        // Move and resize the window to the new position and size
        MoveWindow(hwnd, newLeft, 0, newWidth, rect.bottom - rect.top, TRUE);
    }
}

void moveWindowRight() {
    // Get the handle of the currently active window
    const HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        RECT rect;
        // Get the current position and size of the window
        GetWindowRect(hwnd, &rect);

        // Get the width of the screen
        const int screenWidth = GetSystemMetrics(SM_CXSCREEN);

        // Calculate the new width for the window (half of the screen width)
        const int newWidth = screenWidth / 2;

        // Calculate the new left position for the window
        int newLeft = rect.left + newWidth;

        // Ensure the new left position is not off the screen
        if (newLeft + newWidth > screenWidth) {
            newLeft = screenWidth - newWidth;
        }

        // Move and resize the window to the new position and size
        MoveWindow(hwnd, newLeft, 0, newWidth, rect.bottom - rect.top, TRUE);
    }
}