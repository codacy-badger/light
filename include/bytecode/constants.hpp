#pragma once

#include "util/logger.hpp"

#include <stdint.h>

#define CAPACITY_INCREMENT 256

struct Bytecode_Constants {
    uint8_t* memory = NULL;
    size_t capacity = 0;
    size_t index = 0;

    size_t add (const char* text) {
        auto length = strlen(text);

        while ((this->index + length) > this->capacity) {
            this->capacity += CAPACITY_INCREMENT;

            auto last_memory = this->memory;
            this->memory = (uint8_t*) realloc(this->memory, this->capacity);
            if (last_memory != NULL && this->memory != last_memory) {
                Logger::debug("Realloc memory copy triggered: [ %zd / %zd ]",
                    this->index, this->capacity);
            }
        }

        memcpy(memory + index, text, length);

        auto offset = this->index;
        this->index += length;
        return offset;
    }
};
