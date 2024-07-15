#include <stdio.h>
#include <windows.h>
#include "src/config_parser.h"
#include "src/keyboard.h"

int main() {
    int entryCount = 0;
    ConfigEntry *entries = parseConfigFile("C:\\Users\\photo\\CLionProjects\\wasd_config_parser\\wasd.config", &entryCount);

    printf("Press 'Esc' to exit.\n");

    while (1) {
        keyboardUpdateKeys();

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        const KeyState keyState = getPressedKey();
        if (keyState.isPressed) {
            printf("keycode = %d, isCtrlPressed = %d, isAltPressed = %d, isShiftPressed = %d\n",
                   keyState.keyCode, keyState.isCtrlPressed, keyState.isAltPressed, keyState.isShiftPressed);

            // check if mapping exists in config
            for (int i = 0 ; i < entryCount; i++) {
                if (entries[i].keyCode == keyState.keyCode &&
                    entries[i].altKey == keyState.isAltPressed &&
                    entries[i].ctrlKey == keyState.isCtrlPressed &&
                    entries[i].shiftKey == keyState.isShiftPressed) {
                    printf("MATCHED!\n");
                }
            }
        }

        Sleep(10);
    }

    return 0;
}