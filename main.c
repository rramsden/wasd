#include <stdio.h>
#include <windows.h>
#include "src/config_parser.h"
#include "src/keyboard.h"

int main() {
    int entryCount = 0;
    ConfigEntry *entries = parseConfigFile("C:\\Users\\photo\\CLionProjects\\wasd_config_parser\\wasd.config", &entryCount);

    // Print entries
    for (int i = 0 ; i < entryCount; i++) {
        printf("key1 = %d, key2 = %d, command = %s\n", entries[i].key1, entries[i].key2, entries[i].command);
    }

    printf("Press 'Esc' to exit.\n");

    while (1) {
        keyboardUpdateKeys();

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        if (getPressedKeysCount() > 0) {
            // Seems I do not get consistent results with key codes above
            // when using alt, ctrl, shift, etc.
            const int key1 = getPressedKeys()[0].keyCode;
            const int key2 = getPressedKeysCount() > 1 ? getPressedKeys()[1].keyCode : 0;

            for (int i = 0; i < entryCount; i++) {
                printf("key1 = %d, key2 = %d\n", key1, key2);
                if (key1 == entries[i].key1 && key2 == entries[i].key2) {
                    printf("Command: %s\n", entries[i].command);
                }
            }
        }

        Sleep(10);
    }

    return 0;
}