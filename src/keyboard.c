#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include "keyboard.h"

static KeyState pressedKey;
static bool previousKeyStates[256] = {0};

void resetPressedKeys() {
    pressedKey.keyCode = 0;
    pressedKey.isPressed = false;
    pressedKey.isCtrlPressed = false;
    pressedKey.isAltPressed = false;
    pressedKey.isShiftPressed = false;
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

void keyboardUpdateKeys() {
    bool anyKeyPressed = false;

    for (int key = 8; key <= 255; key++) {
        const SHORT keyState = GetAsyncKeyState(key);
        const bool isCurrentlyPressed = (keyState & 0x8000) != 0;
        const bool wasPreviouslyPressed = previousKeyStates[key];

        // Track previous pressed keys for tracking key up
        previousKeyStates[key] = isCurrentlyPressed;

        if (isCurrentlyPressed) {
            anyKeyPressed = true;
            updateKeysPressed(key);
        } else if (wasPreviouslyPressed) {
            resetPressedKeys();
        }
    }

    if (!anyKeyPressed) {
        resetPressedKeys();
    }
}

KeyState getPressedKey() {
    return pressedKey;
}