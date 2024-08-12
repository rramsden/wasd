//
// Created by photo on 7/28/2024.
//

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <windows.h>
#include <stdbool.h>

#define MAX_WINDOWS 256

typedef struct {
    HWND hwnd;
    RECT rect;
    TCHAR processName[MAX_PATH];
    TCHAR windowTitle[MAX_PATH];
    DWORD processId;
    bool IsIconic; // minimized
    bool IsZoomed; // maximized
    bool IsVisible; // offscreen process
} WindowHandle;

void moveWindowLeft();
void moveWindowRight();
void moveWindowUp();
void moveWindowDown();
void maximizeWindow();
void tileWindowsVertically();
void cycleFocus();

#endif //WINDOW_MANAGER_H
