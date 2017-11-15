#include "bytecode/globals.hpp"

#include <stdio.h>

uint8_t* Bytecode_Globals::get (size_t offset, size_t size) {
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
