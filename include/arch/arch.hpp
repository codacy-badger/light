#pragma once

#include <stdint.h>

enum Arch_Type : uint8_t {
    ARCH_TYPE_UNKNOWN = 0,

    ARCH_TYPE_X64,
};

struct Arch {
    Arch_Type type = ARCH_TYPE_UNKNOWN;
    const char* name = NULL;

    uint8_t register_count = 0;
    uint8_t register_size = 0;

    static Arch* x64;

    Arch (const char* name, Arch_Type type, uint8_t reg_count, uint8_t reg_size) {
        this->name = name;
        this->type = type;
        this->register_count = reg_count;
        this->register_size = reg_size;
    }

    static Arch* get_current_arch ();
};

struct Arch_x64 : Arch {
    Arch_x64 () : Arch ("x64", ARCH_TYPE_X64, 16, 8) { /* empty */ }
};
