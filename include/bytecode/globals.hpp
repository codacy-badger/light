#pragma once

struct Bytecode_Globals {
    uint8_t* memory = NULL;
    size_t capacity = 0;
    size_t index = 0;

    size_t allocate (size_t size) {
        while ((this->index + size) > this->capacity) {
            this->capacity += CAPACITY_INCREMENT;
            this->memory = (uint8_t*) realloc(this->memory, this->capacity);
        }

        auto offset = this->index;
        this->index += size;
        return offset;
    }
};
