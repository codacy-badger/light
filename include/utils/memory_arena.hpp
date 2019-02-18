#pragma once

#define DEFAULT_ARENA_SIZE 1048576

struct Memory_Arena {
    char* data = NULL;
    size_t current_location = 0;

    Memory_Arena () {
        this->data = (char*) malloc(DEFAULT_ARENA_SIZE);
    }

    template<typename T>
    char* get () { return this->get(sizeof(T)); }

    char* get (size_t size) {
        assert((this->current_location + size) < DEFAULT_ARENA_SIZE);

        auto result = this->data + this->current_location;
        this->current_location += size;
        return result;
    }
};
