#pragma once

#include <map>

struct Bytecode_Globals {
    std::map<size_t, uint8_t*> already_allocated;

    uint8_t* get (size_t offset, size_t amount);
};
