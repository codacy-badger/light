#pragma once

#include "platform.hpp"

#include <vector>

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
            default:                return NULL;
        }
    }
};

struct Build_Settings {
	const char* output = NULL;
	std::vector<const char*> input_files;

	char initial_path[MAX_PATH_LENGTH];

	bool is_multithread = false;
	bool is_debug = false;

	OS* target_os = OS::get_current_os();
	Arch* target_arch = Arch::get_current_arch();

	Build_Settings () {
		os_get_current_directory(this->initial_path);
	}
};
