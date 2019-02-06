#pragma once

struct Token_Location {
    const char* absolute_path = NULL;
    size_t line = 0;
    size_t col_begin = 0;
    size_t col_end = 0;
};
