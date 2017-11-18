#pragma once

#include <vector>

struct Bytecode_Constants {
    std::vector<uint8_t*> allocated;

    size_t add (char* string);
};
