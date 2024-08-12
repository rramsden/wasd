#include <stdio.h>
#include <windows.h>
#include "config_parser.h"
#include "keyboard.h"

#include "window_manager.h"

#define DEBOUNCE_TIME 100

static ConfigEntry *entries = NULL;
static int entryCount = 0;
static DWORD lastMatchTime = 0;

int onKeyEvent(const KeyState *keyState) {
    printf("key: %d, alt: %d, ctrl: %d, shift: %d\n", keyState->keyCode, keyState->isAltPressed, keyState->isCtrlPressed, keyState->isShiftPressed);
    const DWORD currentTime = GetTickCount();

    // check if mapping exists in config
    for (int i = 0 ; i < entryCount; i++) {
        // Debounce to prevent multiple matches
        if (currentTime - lastMatchTime < DEBOUNCE_TIME) {
            return 0;
        }

        if (entries[i].keyCode == keyState->keyCode &&
            entries[i].altKey == keyState->isAltPressed &&
            entries[i].ctrlKey == keyState->isCtrlPressed &&
            entries[i].shiftKey == keyState->isShiftPressed) {
            lastMatchTime = GetTickCount();
            printf("EXECUTE: %s\n", entries[i].command);

            if (strcmp(entries[i].command, "window move left") == 0) {
                moveWindowLeft();
            } else if (strcmp(entries[i].command, "window move right") == 0) {
                moveWindowRight();
            } else if (strcmp(entries[i].command, "window move up") == 0) {
                moveWindowUp();
            } else if (strcmp(entries[i].command, "window move down") == 0) {
                moveWindowDown();
            } else if (strcmp(entries[i].command, "cycle focus") == 0) {
                cycleFocus();
            } else if (strcmp(entries[i].command, "window maximize") == 0) {
                maximizeWindow();
            } else if (strcmp(entries[i].command, "window tile vertical") == 0) {
                tileWindowsVertically();
            }

            return 1; // stop propogation
        }
    }

    return 0;
}

int main() {
    printf("Press ESC to exit\n");

    registerKeyEventCallback(onKeyEvent);
    entries = parseConfigFile("../wasd.config", &entryCount);

    startMessageLoop();

    return 0;
}