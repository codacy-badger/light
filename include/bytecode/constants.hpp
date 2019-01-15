#pragma once

#include <stdint.h>

struct Bytecode_Constants {
    uint8_t* memory = NULL;
    size_t capacity = 0;
    size_t index = 0;

    size_t add (const char* text) {
        auto length = strlen(text);

        if ((this->index + length) > this->capacity) {
            this->capacity *= 2;
            realloc(this->memory, this->capacity);
        }

        memcpy(memory + index, text, length);

        auto offset = this->index;
        this->index += length;
        return offset;
    }
};
