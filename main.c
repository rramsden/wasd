#include <stdio.h>
#include <windows.h>
#include "src/config_parser.h"
#include "src/keyboard.h"

#define DEBOUNCE_TIME 300

static ConfigEntry *entries = NULL;
static int entryCount = 0;
static DWORD lastMatchTime = 0;

void onKeyEvent(KeyState keyState) {
    const DWORD currentTime = GetTickCount();

    // check if mapping exists in config
    for (int i = 0 ; i < entryCount; i++) {
        // Debounce to prevent multiple matches
        if (currentTime - lastMatchTime < DEBOUNCE_TIME) {
            continue;
        }

        if (entries[i].keyCode == keyState.keyCode &&
            entries[i].altKey == keyState.isAltPressed &&
            entries[i].ctrlKey == keyState.isCtrlPressed &&
            entries[i].shiftKey == keyState.isShiftPressed) {

            lastMatchTime = GetTickCount();
            printf("MATCHED!\n");
        }
    }
}

int main() {
    printf("Press ESC to exit\n");

    registerKeyEventCallback(onKeyEvent);

    entries = parseConfigFile("../wasd.config", &entryCount);

    while (1) {
        keyboardUpdateKeys();

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        Sleep(10);
    }

    return 0;
}