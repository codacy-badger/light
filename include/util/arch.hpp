#pragma once

#include "report.hpp"
#include "platform.hpp"

struct Arch {
    Arch_Type type;
    const char* name;

    uint8_t register_count;
    uint8_t register_size;

    Arch (const char* name, Arch_Type type, uint8_t reg_count, uint8_t reg_size) {
        this->name = name;
        this->type = type;
        this->register_count = reg_count;
        this->register_size = reg_size;
    }

    static Arch* get_current_arch () {
        auto current_arch = os_get_arch();
        switch (current_arch) {
            case ARCH_TYPE_X64:     return new Arch ("x64", ARCH_TYPE_X64, 16, 8);
            default:                report_error(NULL, "Unknown target arch");
        }
        return NULL;
    }
};
