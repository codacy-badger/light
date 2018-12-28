#pragma once

#include "ast/ast.hpp"

struct Ast_Factory {
    uint64_t node_count = 0;

    Location* location = NULL;

    template<typename T, typename ... Arguments>
    T* create (Arguments ... args) {
        Ast_Factory::node_count++;
        auto node = new T(args...);
        if (location != NULL) {
            node->location = (*this->location);
        }
        return node;
    }

    void destroy (Ast* node) {
        Ast_Factory::node_count--;
        delete node;
    }
};
