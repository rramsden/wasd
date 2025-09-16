#include "window_manager.h"
#include "grid.h"
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <tchar.h>
#include <tlhelp32.h>

static WindowHandle windowHandles[MAX_WINDOWS];
static int currentWindowIndex = 0;
volatile static int windowCount = 0;

// Used to remember the order of windows
static int windowOrdinals[MAX_WINDOWS];
void initializeWindowOrdinals() {
  for (int i = 0; i < MAX_WINDOWS; i++) {
    windowOrdinals[i] = -1;
  }
}

// Exception list for processes that are not spawned by explorer.exe
static const TCHAR* exclusionList[] = {
  TEXT("discord.exe"),
  TEXT("explorer.exe"),
  TEXT("spotify.exe"),
};

static BOOL _IsProcessInExclusionList(const TCHAR* processName)
{
  for (int i = 0; i < sizeof(exclusionList) / sizeof(exclusionList[0]); i++)
  {
    if (_tcsicmp(processName, exclusionList[i]) == 0)
    {
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL _IsProcessSpawnedByExplorer(const DWORD processId)
{
  BOOL result = FALSE;
  const HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
  if (hProcess)
  {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
      if (Process32First(hSnapshot, &pe32))
      {
        do
        {
          if (pe32.th32ProcessID == processId)
          {
            const DWORD parentProcessId = pe32.th32ParentProcessID;
            const HANDLE hParentProcess = OpenProcess(
              PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, parentProcessId);

            if (hParentProcess)
            {
              TCHAR processName[MAX_PATH] = TEXT("<unknown>");
              TCHAR parentProcessName[MAX_PATH] = TEXT("<unknown>");

              GetModuleBaseName(hParentProcess, NULL, parentProcessName, sizeof(parentProcessName) / sizeof(TCHAR));

              if (_tcsicmp(parentProcessName, TEXT("explorer.exe")) == 0)
              {
                result = TRUE;
              }
              CloseHandle(hParentProcess);
            }
            break;
          }
        }
        while (Process32Next(hSnapshot, &pe32));
      }
      CloseHandle(hSnapshot);
    }
    CloseHandle(hProcess);
  }
  return result;
}

// Comparison function for qsort
static int compareWindowHandles(const void *a, const void *b) {
  WindowHandle *handleA = (WindowHandle *)a;
  WindowHandle *handleB = (WindowHandle *)b;
  return handleA->orderIndex - handleB->orderIndex;
}

// Function to sort windowHandles array by orderIndex
static void sortWindowHandlesByOrderIndex() {
  qsort(windowHandles, windowCount, sizeof(WindowHandle), compareWindowHandles);
}

static BOOL CALLBACK _EnumWindowProc(const HWND hwnd, LPARAM lparam)
{
  const BOOL ignoreIconic = (BOOL)lparam;
  const HWND focusedWindow = GetForegroundWindow();

  if (IsWindowVisible(hwnd) && (!ignoreIconic || !IsIconic(hwnd)) && windowCount < MAX_WINDOWS)
  {
    const LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    if (!(exStyle & WS_EX_TOOLWINDOW))
    {
      DWORD processId;
      GetWindowThreadProcessId(hwnd, &processId);
      const HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
      if (hProcess)
      {
        TCHAR processName[MAX_PATH] = TEXT("<unknown>");
        TCHAR windowTitle[MAX_PATH] = TEXT("<unknown>");

        GetModuleBaseName(hProcess, NULL, processName, sizeof(processName) / sizeof(TCHAR));
        GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(TCHAR));

        CloseHandle(hProcess);

        // Exclude processes with no window title - these are usually system processes or the toolbar
        if (_tcslen(windowTitle) == 0)
        {
          return TRUE;
        }

        if (_IsProcessInExclusionList(processName) || _IsProcessSpawnedByExplorer(processId))
        {
          windowHandles[windowCount].hwnd = hwnd;
          _tcscpy(windowHandles[windowCount].processName, processName);
          _tcscpy(windowHandles[windowCount].windowTitle, windowTitle);
          windowHandles[windowCount].processId = processId;
          windowHandles[windowCount].IsIconic = IsIconic(hwnd);
          windowHandles[windowCount].IsZoomed = IsZoomed(hwnd);
          windowHandles[windowCount].IsVisible = IsWindowVisible(hwnd);
          windowHandles[windowCount].IsFocused = hwnd == focusedWindow;
          if (windowOrdinals[windowCount] == -1) {
            windowOrdinals[windowCount] = windowCount;
          }
          windowHandles[windowCount].orderIndex = windowOrdinals[windowCount];
          GetWindowRect(hwnd, &windowHandles[windowCount].rect);
          windowCount++;
        }
      }
    }
  }
  return TRUE;
}

static void _bringWindowToFront(const HWND hwnd)
{
  if (IsIconic(hwnd))
  {
    ShowWindow(hwnd, SW_RESTORE);
  }
  SetForegroundWindow(hwnd);
  BringWindowToTop(hwnd);
}

static int _screenWidth()
{
  RECT workArea;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

  return workArea.right - workArea.left;
}

static int _screenHeight()
{
  RECT workArea;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

  return workArea.bottom - workArea.top;
}

void _printWindowHandles()
{
  /*
  printf("Screenwidth %d, ScreenHeight %d\n", _screenWidth(), _screenHeight());
  for (int i = 0; i < windowCount; i++)
  {
    const WindowHandle handle = windowHandles[i];
    const LONG exStyle = GetWindowLong(handle.hwnd, GWL_EXSTYLE);

    printf("Window %d: %s - %s, IsIconic: %d, IsZoomed: %d, IsVisible: %d, IsFocused: %d, Rect: (%ld,%ld,%ld,%ld)\n", i,
           handle.processName, handle.windowTitle, handle.IsIconic, handle.IsZoomed, handle.IsVisible, handle.IsFocused, handle.rect.left,
           handle.rect.top, handle.rect.right, handle.rect.bottom);
  }
  */

  printf("[");
  for (int i = 0; i < windowCount; i++)
  {
    printf("%lu (%d, %d)", windowHandles[i].processId, windowOrdinals[i], windowHandles[i].orderIndex);
    if (i < windowCount - 1) { printf(", "); }
  }
  printf("]\n");
}

void _printWindowOrdinals()
{
  printf("Ordering [");
  for (int i = 0; i < windowCount; i++)
  {
    printf("%d", windowOrdinals[i]);
    if (i < windowCount - 1) { printf(", "); }
  }
  printf("]\n");
}

void _enumWindows(BOOL ignoreIconic)
{
  windowCount = 0;
  EnumWindows(_EnumWindowProc, (LPARAM)ignoreIconic);
  sortWindowHandlesByOrderIndex();
  _printWindowHandles();
  _printWindowOrdinals();
}

// Function to raise and focus a window
void _RaiseAndFocusWindow(const HWND hwnd)
{
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

void cycleFocus()
{
  if (windowCount > 0)
  {
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

    printf("Current window: %s - %s\n", windowHandles[currentWindowIndex].processName,
           windowHandles[currentWindowIndex].windowTitle);
  }
}

void tileWindowsVertically()
{
  _enumWindows(true);

  if (windowCount == 0) {
    return;
  }

  const Grid *grid = grid_assign_handles(&windowHandles, windowCount);

  const int screenWidth = _screenWidth();
  const int screenHeight = _screenHeight();

  for (int w = 0; w < windowCount; w++) {
    const WindowHandle *handler = &windowHandles[w];
    const GridCoordinate coordinate = grid_find_window(grid, handler);

    if (coordinate.x == -1 || coordinate.y == -1) {
      continue;
    }

    const HWND hwnd = handler->hwnd;

    int count_w = 1; // Number of windows in the row
    int pos_w = 0; // Position of the window in the row
    int count_h = 1; // Number of windows in the column
    int pos_h = 0; // Position of the window in the column

    {
      // Count horizontally
      for (int k = 0; k < GRID_COLS; k++) {
        const WindowHandle *window = grid->windows[coordinate.x][k];

        if (window != NULL) {
          if (window->processId != handler->processId) {
            count_w++;
          }

          if (window != handler && k < coordinate.y) {
            pos_w++;
          }
        }
      }

      // Count vertically
      for (int k = 0; k < GRID_ROWS; k++) {
        const WindowHandle *window = grid->windows[k][coordinate.y];

        if (window != NULL) {
          if (window->processId != handler->processId) {
            count_h++;
          }

          if (window != handler && k < coordinate.x) {
            pos_h++;
          }
        }
      }
    }

    const int window_width = screenWidth / count_w;
    const int window_height = screenHeight / count_h;
    const int new_x = pos_w * window_width;
    const int new_y = pos_h * window_height;

    printf("id: %lu, pos_w: %d, pos_h: %d, count_w: %d, count_h: %d, new_x: %d, new_y: %d, window_width: %d, window_height: %d\n",
    handler->processId, pos_w, pos_h, count_w, count_h, new_x, new_y, window_width, window_height);

    // Restore the window if it is maximized
    if (handler->IsZoomed) {
      ShowWindow(hwnd, SW_RESTORE);
    }

    _bringWindowToFront(hwnd);

    MoveWindow(hwnd, new_x, new_y, window_width, window_height, TRUE);
  }
}

void maximizeWindow()
{
  const HWND hwnd = GetForegroundWindow();

  if (hwnd)
  {
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

    MoveWindow(hwnd, workArea.left, workArea.top, _screenWidth(), _screenHeight(), TRUE);
  }
}

static void updateWindowOrder(const int direction, const HWND hwnd) {
  for (int i = 0; i < windowCount; i++) {
    if (windowHandles[i].hwnd == hwnd) {
      const int newIndex = i + direction;
      if (newIndex >= 0 && newIndex < windowCount) {
        printf("Swapping: %d and %d\n", windowOrdinals[i], windowOrdinals[newIndex]);
        const int temp = windowOrdinals[i];
        windowOrdinals[i] = windowOrdinals[newIndex];
        windowOrdinals[newIndex] = temp;

        _printWindowOrdinals();
        break;
      }
    }
  }
}


void moveWindowLeft()
{
  const HWND hwnd = GetForegroundWindow();

  if (hwnd)
  {
    updateWindowOrder(-1, hwnd);
    tileWindowsVertically();
  }
}


void moveWindowRight()
{
  const HWND hwnd = GetForegroundWindow();

  if (hwnd)
  {
    updateWindowOrder(1, hwnd);
    tileWindowsVertically();
  }
}
