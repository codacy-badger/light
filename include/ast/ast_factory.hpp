#pragma once

#include "lexer/lexer.hpp"
#include "ast/ast.hpp"

struct Ast_Factory {
    Lexer* lexer = NULL;
    uint64_t node_count = 0;

    template<typename T, typename ... Arguments>
    T* create_node (Arguments ... args) {
        T* node = new T(args...);
        if (this->lexer) {
            node->location = this->lexer->buffer->location;
        }
        this->node_count++;
        return node;
    }

    void delete_node (Ast* node) {
        this->node_count--;
        delete node;
    }
};
