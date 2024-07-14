#ifndef WASD_CONFIG_PARSER_LIBRARY_H
#define WASD_CONFIG_PARSER_LIBRARY_H

#define MAX_LINE_LENGTH 1024
#define MAX_ENTRIES 1024

typedef struct ConfigEntry {
    int key1;
    int key2;
    int key3;
    char command[MAX_LINE_LENGTH];
} ConfigEntry;

ConfigEntry* parseConfigFile(const char* filename, int* entryCount);

#endif WASD_CONFIG_PARSER_LIBRARY_H
