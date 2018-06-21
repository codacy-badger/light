#pragma once

#include "lexer/lexer.hpp"
#include "parser/ast.hpp"

struct Ast_Factory {
    Lexer* lexer = NULL;

    template<typename T, typename ... Arguments>
    T* new_node (Arguments ... args) {
        T* node = new T(args...);
        if (this->lexer) {
            node->location = this->lexer->buffer->location;
        }
        return node;
    }
};
