#pragma once

#include <stdio.h>
#include <map>

struct Bytecode_Globals {
    std::map<size_t, uint8_t*> already_allocated;

    uint8_t* get (size_t offset, size_t size) {
        auto it = this->already_allocated.find(offset);
        if (it != this->already_allocated.end()) {
            return this->already_allocated[offset];
        } else {
            printf("Allocating global variable at %zd (size %zd)\n", offset, size);
            uint8_t* ptr = (uint8_t*) malloc(size);
            this->already_allocated[offset] = ptr;
            return ptr;
        }
    }
};
