#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

#define MAX_KEYS 25
typedef struct {
    int keyCode;
    bool isPressed;
    bool isCtrlPressed;
    bool isAltPressed;
    bool isShiftPressed;
} KeyState;

typedef void (*KeyEventCallback)(KeyState keyState);
void registerKeyEventCallback(KeyEventCallback callback);

void keyboardUpdateKeys();
KeyState getPressedKey();

#endif //KEYBOARD_H
