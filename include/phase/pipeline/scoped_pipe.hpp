#pragma once

#include "pipe.hpp"

struct Scoped_Pipe : Pipe {
    Ast_Scope* current_scope = NULL;

    void handle (Ast_Scope** block_ptr) {
        auto tmp = this->current_scope;
        this->current_scope = (*block_ptr);
        Pipe::handle(block_ptr);
        this->current_scope = tmp;
    }
};
