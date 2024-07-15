#ifndef WASD_CONFIG_PARSER_LIBRARY_H
#define WASD_CONFIG_PARSER_LIBRARY_H

#include <stdbool.h>

#define MAX_LINE_LENGTH 1024
#define MAX_ENTRIES 1024

typedef struct ConfigEntry {
    int keyCode;

    bool ctrlKey;
    bool altKey;
    bool shiftKey;

    char command[MAX_LINE_LENGTH];
} ConfigEntry;

ConfigEntry* parseConfigFile(const char* filename, int* entryCount);

#endif WASD_CONFIG_PARSER_LIBRARY_H
