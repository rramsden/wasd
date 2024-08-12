#include "window_manager.h"
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <tchar.h>
#include <tlhelp32.h>

static WindowHandle windowHandles[MAX_WINDOWS];
volatile static int windowCount = 0;
static int currentWindowIndex = 0;

// Exception list for processes that are not spawned by explorer.exe
static const TCHAR* exclusionList[] = {
    TEXT("discord.exe"),
    TEXT("explorer.exe")
};

static BOOL _IsProcessInExclusionList(const TCHAR* processName) {
    for (int i = 0; i < sizeof(exclusionList) / sizeof(exclusionList[0]); i++) {
        if (_tcsicmp(processName, exclusionList[i]) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL _IsProcessSpawnedByExplorer(const DWORD processId) {
    BOOL result = FALSE;
    const HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            if (Process32First(hSnapshot, &pe32)) {
                do {
                    if (pe32.th32ProcessID == processId) {
                        const DWORD parentProcessId = pe32.th32ParentProcessID;
                        const HANDLE hParentProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, parentProcessId);
                        if (hParentProcess) {
                            TCHAR parentProcessName[MAX_PATH] = TEXT("<unknown>");

                            GetModuleBaseName(hParentProcess, NULL, parentProcessName, sizeof(parentProcessName) / sizeof(TCHAR));

                            if (_tcsicmp(parentProcessName, TEXT("explorer.exe")) == 0) {
                                result = TRUE;
                            }
                            CloseHandle(hParentProcess);
                        }
                        break;
                    }
                } while (Process32Next(hSnapshot, &pe32));
            }
            CloseHandle(hSnapshot);
        }
        CloseHandle(hProcess);
    }
    return result;
}

static BOOL CALLBACK _EnumWindowProc(const HWND hwnd, LPARAM lparam) {
    if (IsWindowVisible(hwnd) && windowCount < MAX_WINDOWS) {
        const LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

        if (!(exStyle & WS_EX_TOOLWINDOW)) {
            DWORD processId;
            GetWindowThreadProcessId(hwnd, &processId);

            const HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
            if (hProcess) {
                TCHAR processName[MAX_PATH] = TEXT("<unknown>");
                TCHAR windowTitle[MAX_PATH] = TEXT("<unknown>");

                GetModuleBaseName(hProcess, NULL, processName, sizeof(processName) / sizeof(TCHAR));
                GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(TCHAR));

                CloseHandle(hProcess);

                // Exclude processes with no window title - these are usually system processes or the toolbar
                if (_tcslen(windowTitle) == 0) {
                    return TRUE;
                }

                if (_IsProcessInExclusionList(processName) || _IsProcessSpawnedByExplorer(processId)) {
                    windowHandles[windowCount].hwnd = hwnd;
                    _tcscpy(windowHandles[windowCount].processName, processName);
                    _tcscpy(windowHandles[windowCount].windowTitle, windowTitle);
                    windowHandles[windowCount].processId = processId;
                    windowHandles[windowCount].IsIconic = IsIconic(hwnd);
                    windowHandles[windowCount].IsZoomed = IsZoomed(hwnd);
                    windowHandles[windowCount].IsVisible = IsWindowVisible(hwnd);
                    GetWindowRect(hwnd, &windowHandles[windowCount].rect);
                    windowCount++;
                }
            }
        }
    }
    return TRUE;
}

static void _bringWindowToFront(const HWND hwnd) {
    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }
    SetForegroundWindow(hwnd);
    BringWindowToTop(hwnd);
}

static int _screenWidth() {
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

    return workArea.right - workArea.left;
}

static int _screenHeight() {
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

    return workArea.bottom - workArea.top;
}

void _printWindowHandles() {
    printf("Screenwidth %d, ScreenHeight %d\n", _screenWidth(), _screenHeight());
    for (int i = 0; i < windowCount; i++) {
        const WindowHandle handle = windowHandles[i];
        const LONG exStyle = GetWindowLong(handle.hwnd, GWL_EXSTYLE);

        printf("Window %d: %s - %s, IsIconic: %d, IsZoomed: %d, IsVisible: %d, Rect: (%ld,%ld,%ld,%ld)\n", i, handle.processName, handle.windowTitle, handle.IsIconic, handle.IsZoomed, handle.IsVisible, handle.rect.left, handle.rect.top, handle.rect.right, handle.rect.bottom);
    }
}

void _enumWindows() {
    windowCount = 0;
    EnumWindows(_EnumWindowProc, 0);
    _printWindowHandles();
}

// Function to raise and focus a window
void _RaiseAndFocusWindow(const HWND hwnd) {
    // Create an input event for a mouse input
    INPUT event;
    event.type = INPUT_MOUSE;
    event.mi.dx = 0;
    event.mi.dy = 0;
    event.mi.mouseData = 0;
    event.mi.dwFlags = 0;
    event.mi.time = 0;
    event.mi.dwExtraInfo = 0;

    // Send an input event to our own process first so that we pass the foreground lock check
    SendInput(1, &event, sizeof(INPUT));

    // Error ignored, as the operation is not always necessary.
    SetWindowPos(
        hwnd,
        HWND_TOP,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW
    );

    SetForegroundWindow(hwnd);
}

void cycleFocus() {
    if (windowCount > 0) {
        currentWindowIndex = (currentWindowIndex + 1) % windowCount;
        const HWND hwnd = windowHandles[currentWindowIndex].hwnd;

        // Bring the window to the top of the Z order
        _bringWindowToFront(hwnd);

        // Set the window to the foreground
        SetForegroundWindow(hwnd);

        // Set the window focus
        _RaiseAndFocusWindow(hwnd);

        // Emulate a mouse action on the window to ensure it is focused
        RECT rect;
        GetWindowRect(hwnd, &rect);
        SetCursorPos((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);

        printf("Current window: %s - %s\n", windowHandles[currentWindowIndex].processName, windowHandles[currentWindowIndex].windowTitle);
    }
}

void tileWindowsVertically() {
    _enumWindows();

    if (windowCount == 0) {
        return;
    }

    const int screenWidth = _screenWidth();
    const int screenHeight = _screenHeight();
    const int windowWidth = screenWidth / windowCount;

    for (int i = 0; i < windowCount; i++) {
        const WindowHandle handler = windowHandles[i];
        const HWND hwnd = handler.hwnd;

        if (hwnd) {
            if (handler.IsZoomed) {
                ShowWindow(hwnd, SW_RESTORE);
            }

            _bringWindowToFront(hwnd);

            MoveWindow(hwnd, i * windowWidth, 0, windowWidth, screenHeight, TRUE);
        }
    }
}

void maximizeWindow() {
    const HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        RECT workArea;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

        MoveWindow(hwnd, workArea.left, workArea.top, _screenWidth(), _screenHeight(), TRUE);
    }
}

void moveWindowUp() {
    const HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        if (IsZoomed(hwnd)) {
            ShowWindow(hwnd, SW_RESTORE);
        }

        RECT rect;
        GetWindowRect(hwnd, &rect);

        const int newHeight = _screenHeight() / 2;

        MoveWindow(hwnd, rect.left, 0, rect.right - rect.left, newHeight, TRUE);
    }
}

void moveWindowDown() {
    const HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        RECT rect;
        GetWindowRect(hwnd, &rect);

        const int newHeight = _screenHeight() / 2;

        MoveWindow(hwnd, rect.left, newHeight, rect.right - rect.left, newHeight, TRUE);
    }
}

void moveWindowLeft() {
    const HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        if (IsZoomed(hwnd)) {
            ShowWindow(hwnd, SW_RESTORE);
        }

        RECT rect;
        GetWindowRect(hwnd, &rect);

        const int newWidth = _screenWidth() / 2;
        int newLeft = rect.left - newWidth;

        if (newLeft < 0) {
            newLeft = 0;
        }

        MoveWindow(hwnd, newLeft, 0, newWidth, _screenHeight(), TRUE);
    }
}

void moveWindowRight() {
    const HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        if (IsZoomed(hwnd)) {
            ShowWindow(hwnd, SW_RESTORE);
        }

        RECT rect;
        GetWindowRect(hwnd, &rect);

        const int newWidth = _screenWidth() / 2;
        int newLeft = rect.left + newWidth;

        if (newLeft + newWidth > _screenWidth()) {
            newLeft = _screenWidth() - newWidth;
        }

        MoveWindow(hwnd, newLeft, 0, newWidth, _screenHeight(), TRUE);
    }
}