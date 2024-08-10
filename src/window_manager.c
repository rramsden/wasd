#include "window_manager.h"
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <tchar.h>

static HWND windowHandles[MAX_WINDOWS];
static int windowCount = 0;
static int currentWindowIndex = 0;

static const TCHAR* _excludedApplications[] = {
    TEXT("ApplicationFrameHost.exe"),
    TEXT("TextInputHost.exe"),
    TEXT("Explorer.EXE")
};

static BOOL _isExcludedApplication(const TCHAR* processName) {
    for (int i = 0; i < sizeof(_excludedApplications) / sizeof(TCHAR*); i++) {
        if (_tcscmp(processName, _excludedApplications[i]) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL CALLBACK _EnumWindowProc(HWND hwnd, LPARAM lparam) {
    if (IsWindowVisible(hwnd) && windowCount < MAX_WINDOWS) {
        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

        if (!(exStyle & WS_EX_TOOLWINDOW)) {
            DWORD processId;
            GetWindowThreadProcessId(hwnd, &processId);
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
            if (hProcess) {
                TCHAR processName[MAX_PATH] = TEXT("<unknown>");
                GetModuleBaseName(hProcess, NULL, processName, sizeof(processName) / sizeof(TCHAR));
                CloseHandle(hProcess);

                if (!_isExcludedApplication(processName)) {
                    windowHandles[windowCount++] = hwnd;
                }
            }
        }
    }
    return TRUE;
}

void initWindowManager() {
    if (windowCount == 0) {
        EnumWindows(_EnumWindowProc, 0);
    }
}

static void bringWindowToFront(HWND hwnd) {
    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }
    SetForegroundWindow(hwnd);
    BringWindowToTop(hwnd);
}

void cycleFocus() {
    if (windowCount > 0) {
        currentWindowIndex = (currentWindowIndex + 1) % windowCount;
        const HWND hwnd = windowHandles[currentWindowIndex];

        // Bring the window to the top of the Z order
        bringWindowToFront(hwnd);

        // Set the window to the foreground
        SetForegroundWindow(hwnd);

        // Emulate a mouse click on the window to ensure it is focused
        RECT rect;
        GetWindowRect(hwnd, &rect);
        SetCursorPos((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

        // Get the process ID
        DWORD processId;
        GetWindowThreadProcessId(hwnd, &processId);

        // Open the process
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
        if (hProcess) {
            // Get the process name
            TCHAR processName[MAX_PATH] = TEXT("<unknown>");
            GetModuleBaseName(hProcess, NULL, processName, sizeof(processName) / sizeof(TCHAR));

            // Get the window title
            TCHAR windowTitle[MAX_PATH];
            GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(TCHAR));

            // Print the process name and window title
            printf("Focused on process: %s, window title: %s\n", processName, windowTitle);

            // Close the process handle
            CloseHandle(hProcess);
        }
    }
}

void moveWindowUp() {
    const HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        RECT rect;
        GetWindowRect(hwnd, &rect);

        const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        const int newHeight = screenHeight / 2;

        MoveWindow(hwnd, rect.left, 0, rect.right - rect.left, newHeight, TRUE);
    }
}

void moveWindowDown() {
    const HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        RECT rect;
        GetWindowRect(hwnd, &rect);

        const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        const int newHeight = screenHeight / 2;

        MoveWindow(hwnd, rect.left, newHeight, rect.right - rect.left, newHeight, TRUE);
    }
}

// Function to move the currently active window to the left half of the screen
void moveWindowLeft() {
    const HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        RECT rect;
        GetWindowRect(hwnd, &rect);

        const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        const int newWidth = screenWidth / 2;
        int newLeft = rect.left - newWidth;

        if (newLeft < 0) {
            newLeft = 0;
        }

        MoveWindow(hwnd, newLeft, 0, newWidth, screenHeight, TRUE);
    }
}

void moveWindowRight() {
    const HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        RECT rect;
        GetWindowRect(hwnd, &rect);

        const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        const int newWidth = screenWidth / 2;
        int newLeft = rect.left + newWidth;

        if (newLeft + newWidth > screenWidth) {
            newLeft = screenWidth - newWidth;
        }

        MoveWindow(hwnd, newLeft, 0, newWidth, screenHeight, TRUE);
    }
}