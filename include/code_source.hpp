#pragma once

#include "ast/nodes.hpp"

struct Code_Source {
    const char* path = NULL;
    const char* from = NULL;

    char* absolute_path = NULL;

    const char* text = NULL;
    size_t length = 0;

    Ast_Scope* import_into = NULL;

    Code_Source (const char* path, const char* from = NULL) {
        this->path = path;
        this->from = from;
    }

    Code_Source (const char* text, size_t length) {
        this->text = text;
        this->length = length;
    }
};
