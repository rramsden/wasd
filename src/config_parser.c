#include "config_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static char* trimWhitespace(char* str);
static char* stripComment(char* str);
static void parseKeyCombination(ConfigEntry* entry, const char* keyCombination);

ConfigEntry* parseConfigFile(const char* filePath, int *entryCount) {
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    ConfigEntry* entries = malloc(sizeof(ConfigEntry) * MAX_ENTRIES);
    char line[MAX_LINE_LENGTH];
    int count = 0;

    while (fgets(line, MAX_LINE_LENGTH, file)) {
        // Ignore comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;

        // Find the colon seperating key combination and command
        char* colonPos = strchr(line, ':');
        if (colonPos == NULL) continue; // Skip line if no colon is found

        *colonPos = '\0'; // Split the line into two strings

        char *keyCombination = trimWhitespace(line);
        char *command = trimWhitespace(stripComment(colonPos + 1));

        ConfigEntry entry = {0,false, false, false, ""};
        strcpy(entry.command, command);
        parseKeyCombination(&entry, keyCombination);

        entries[count++] = entry;
    }

    fclose(file);
    *entryCount = count;

    return entries;
}

// Function to map key names to virtual key codes
static int mapKeyToVirtualKeyCode(const char* key) {
    if (strcmp(key, "ctrl") == 0) return VK_CONTROL;
    else if (strcmp(key, "alt") == 0) return VK_MENU;
    else if (strcmp(key, "shift") == 0) return VK_SHIFT;
    else return toupper(key[0]); // Assuming single character keys are letters
}

static char* trimWhitespace(char* str) {
    int start = 0, end = strlen(str) - 1;
    while (str[start] != '\0' && isspace((unsigned char)str[start])) {
        start++;
    }
    while (end >= start && isspace((unsigned char)str[end])) {
        end--;
    }
    int j = 0;
    for (int i = start; i <= end; i++) {
        str[j++] = str[i];
    }
    str[j] = '\0'; // Null-terminate the string

    return str;
}

static char* stripComment(char* str) {
    char* hashPos = strchr(str, '#');
    if (hashPos != NULL) {
        *hashPos = '\0';
    }
    return str;
}

// Function to parse a key combination string and fill a ConfigEntry with virtual key codes
static void parseKeyCombination(ConfigEntry* entry, const char* keyCombination) {
    char buffer[MAX_LINE_LENGTH];
    strcpy(buffer, keyCombination); // Copy to a buffer to use strtok without modifying original string

    char* token = strtok(buffer, "+");
    int keyIndex = 0;
    while (token != NULL && keyIndex < 3) { // Assuming up to three keys
        const int vkCode = mapKeyToVirtualKeyCode(trimWhitespace(token));

        if (vkCode == VK_SHIFT || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT) {
            entry->shiftKey = true;
        } else if (vkCode == VK_CONTROL || vkCode == VK_LCONTROL || vkCode == VK_RCONTROL) {
            entry->ctrlKey = true;
        } else if (vkCode == VK_MENU || vkCode == VK_LMENU || vkCode == VK_RMENU) {
            entry->altKey = true;
        } else {
            entry->keyCode = vkCode;
        }

        token = strtok(NULL, "+");
        keyIndex++;
    }
}

