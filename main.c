#include <stdio.h>
#include <windows.h>
#include "src/config_parser.h"
#include "src/keyboard.h"

int main() {
    int entryCount = 0;
    ConfigEntry *entries = parseConfigFile("C:\\Users\\photo\\CLionProjects\\wasd_config_parser\\wasd.config", &entryCount);

    for (int i = 0 ; i < entryCount; i++) {
        printf("Key1: %d, Key2: %d, Key3: %d, Command: %s\n", entries[i].key1, entries[i].key2, entries[i].key3, entries[i].command);
    }
    return 0;

    printf("Press 'Esc' to exit.\n");

    while (1) {
        keyboardUpdateKeys();

        if (getPressedKeysCount() > 0) {
            printf("KEYCODE %d", getPressedKeys()[0].keyCode);
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        Sleep(10);
    }

    return 0;
}