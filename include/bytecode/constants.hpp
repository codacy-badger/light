#pragma once

#include <vector>

struct Bytecode_Constants {
    std::vector<void*> allocated;

    size_t add (const char* string) {
        this->allocated.push_back((void*) string);
    	return this->allocated.size() - 1;
    }
};
