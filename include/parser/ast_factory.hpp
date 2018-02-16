#pragma once

#include "lexer/lexer.hpp"
#include "parser/ast.hpp"

#include <stdio.h>
#include <typeinfo>

#define BUFFER_DEFAULT_SIZE 256

struct Ast_Factory {
    Lexer* lexer = NULL;

    Ast_Ident* ident_buffer = NULL;
    size_t ident_buffer_index = 0;

    template<typename T, typename ... Arguments>
    T* new_node (Arguments ... args) {
        return new T(args...);
    }

    Ast_Ident* new_ident (Ast_Block* scope) {
        if (!this->ident_buffer || this->ident_buffer_index >= BUFFER_DEFAULT_SIZE) {
            this->ident_buffer = new Ast_Ident[BUFFER_DEFAULT_SIZE];
            this->ident_buffer_index = 0;
        }

        auto ident = &this->ident_buffer[this->ident_buffer_index++];
        ident->scope = scope;
        return ident;
    }

    void delete_ident (Ast_Ident*) { /* empty */ }
};
