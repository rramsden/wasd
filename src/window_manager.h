//
// Created by photo on 7/28/2024.
//

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#define MAX_WINDOWS 256

void moveWindowLeft();
void moveWindowRight();
void moveWindowUp();
void moveWindowDown();
void maximizeWindow();
void tileWindowsVertically();
void cycleFocus();
void initWindowManager();

#endif //WINDOW_MANAGER_H
