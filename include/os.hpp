#pragma once

struct OS {
    OS_Type type;
    const char* name;

    OS (const char* name, OS_Type type) {
        this->name = name;
        this->type = type;
    }

    static OS* get_current_os () {
        #if defined(_WIN32)
            return new OS ("Windows", OS_TYPE_WINDOWS);
        #elif defined(__linux__)
            return new OS ("Linux", OS_TYPE_LINUX);
        #elif defined(__APPLE__)
            return new OS ("Mac", OS_TYPE_MAC);
        #else
            return NULL;
        #endif
    }
};
