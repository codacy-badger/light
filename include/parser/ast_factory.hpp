#pragma once

#include "lexer/lexer.hpp"
#include "parser/ast.hpp"

struct Ast_Factory {
    Lexer* lexer = NULL;
    uint64_t node_count = 0;

    template<typename T, typename ... Arguments>
    T* new_node (Arguments ... args) {
        T* node = new T(args...);
        if (this->lexer) {
            node->location = this->lexer->buffer->location;
        }
        node_count++;
        return node;
    }
};
