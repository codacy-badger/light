#pragma once

struct Location {
    const char* filename;
    size_t line;

    Location (const char* filename = NULL, size_t line = 0) {
        this->filename = filename;
        this->line = line;
    }
};
