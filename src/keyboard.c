#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <pthread.h>
#include "keyboard.h"

static KeyState pressedKey;
static KeyEventCallback keyEventCallback = NULL;
static HHOOK keyboardHook = NULL;
static pthread_t keyboardHookThread;

void resetPressedKeys(const int keyCode) {
    pressedKey.keyCode = 0;
    pressedKey.isPressed = false;

    if (keyCode == VK_LCONTROL || keyCode == VK_RCONTROL || keyCode == VK_CONTROL) {
        pressedKey.isCtrlPressed = false;
    } else if (keyCode == VK_LMENU || keyCode == VK_RMENU || keyCode == VK_MENU) {
        pressedKey.isAltPressed = false;
    } else if (keyCode == VK_LSHIFT || keyCode == VK_RSHIFT || keyCode == VK_SHIFT) {
        pressedKey.isShiftPressed = false;
    }
}

void updateKeysPressed(const int keyCode) {
    // Update the state of left and right control keys
    if (keyCode == VK_LCONTROL || keyCode == VK_RCONTROL || keyCode == VK_CONTROL) {
        pressedKey.isCtrlPressed = true;
    } else if (keyCode == VK_LMENU || keyCode == VK_RMENU || keyCode == VK_MENU) {
        pressedKey.isAltPressed = true;
    } else if (keyCode == VK_LSHIFT || keyCode == VK_RSHIFT || keyCode == VK_SHIFT) {
        pressedKey.isShiftPressed = true;
    } else {
        pressedKey.keyCode = keyCode;
        pressedKey.isPressed = true;
    }
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
        const int vkCode = p->vkCode;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            updateKeysPressed(vkCode);
            return keyEventCallback(&pressedKey);
        }

        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            resetPressedKeys(vkCode);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void registerKeyEventCallback(const KeyEventCallback callback) {
    keyEventCallback = callback;
}

void* lowLevelKeyboardLoop(void *param) {
    MSG msg;

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (keyboardHook == NULL) {
        printf("Failed to set keyboard hook\n");
    } else {
        printf("Keyboard hook installed\n");
    }

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);

    return NULL;
}

void startMessageLoop() {
    if (keyEventCallback == NULL) {
        return perror("No key event callback registered\n");
    }

    if (pthread_create(&keyboardHookThread, NULL, lowLevelKeyboardLoop, NULL) != 0) {
        printf("Failed to create keyboard hook thread\n");
    } else {
        printf("Keyboard hook thread created\n");
    }

    while(1) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        Sleep(10);
    }
}
