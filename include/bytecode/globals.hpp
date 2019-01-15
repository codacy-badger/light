#pragma once

struct Bytecode_Globals {
    size_t current_size = 0;

    size_t allocate (size_t size) {
        auto offset = this->current_size;
        this->current_size += size;
        return offset;
    }
};
