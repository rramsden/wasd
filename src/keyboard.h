#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

#define MAX_KEYS 25
typedef struct {
    int keyCode;
    bool isPressed;
} KeyState;

void keyboardUpdateKeys();
int getPressedKeysCount();
KeyState* getPressedKeys();

#endif //KEYBOARD_H
