#pragma once

#include <stdint.h>

enum OS_Type : uint8_t {
    OS_TYPE_UNKNOWN = 0,

    OS_TYPE_WINDOWS,
    OS_TYPE_LINUX,
    OS_TYPE_MAC,
};

struct OS {
    OS_Type type = OS_TYPE_UNKNOWN;
    const char* name = NULL;

    static OS* windows;
    static OS* linux;
    static OS* mac;

    OS (const char* name, OS_Type type) {
        this->name = name;
        this->type = type;
    }

    static OS* get_current_os () {
        #if defined(_WIN32)
            return OS::windows;
        #elif defined(__linux__)
            return OS::linux;
        #elif defined(__APPLE__)
            return OS::mac;
        #else
            return NULL;
        #endif
    }
};

struct Windows : OS {
    Windows () : OS ("Windows", OS_TYPE_WINDOWS) { /* empty */ }
};

struct Linux : OS {
    Linux () : OS ("Linux", OS_TYPE_LINUX) { /* empty */ }
};

struct Mac : OS {
    Mac () : OS ("Mac", OS_TYPE_MAC) { /* empty */ }
};
