#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include "keyboard.h"

static KeyState pressedKeys[MAX_KEYS];
static int pressedKeysCount = 0;

void resetPressedKeys() {
    pressedKeysCount = 0;
}

void updateKeysPressed(const int keyCode, const bool isPressed) {
    if (isPressed) {
        bool found = false;

        // Check if the key is already in the list
        for (int i = 0; i < pressedKeysCount; ++i) {
            if (pressedKeys[i].keyCode == keyCode) {
                found = true;
                break;
            }
        }

        // Add the key to the list if it's not already there
        if (!found) {
            pressedKeys[pressedKeysCount++] = (KeyState){keyCode, true};
        }
    } else {
        // Remove key from pressedKeys
        for (int i = 0; i < pressedKeysCount; ++i) {
            if (pressedKeys[i].keyCode == keyCode) {
                pressedKeys[i] = pressedKeys[--pressedKeysCount];
                break;
            }
        }
    }
}

void keyboardUpdateKeys() {
    bool anyKeyPressed = false;

    for (int key = 8; key <= 255; key++) {
        SHORT keyState = GetAsyncKeyState(key);
        bool isCurrentlyPressed = (keyState & 0x8000) != 0;

        if (isCurrentlyPressed) {
            anyKeyPressed = true;
            updateKeysPressed(key, isCurrentlyPressed);
        }
    }

    // If no keys are pressed, reset the buffer
    if (!anyKeyPressed) {
        resetPressedKeys();
    }

    // Print currently pressed keys
    for (int i = 0; i < pressedKeysCount; ++i) {
        printf("Key pressed: %d\n", pressedKeys[i].keyCode);
    }
}

int getPressedKeysCount() {
    return pressedKeysCount;
}

KeyState* getPressedKeys() {
    return pressedKeys;
}