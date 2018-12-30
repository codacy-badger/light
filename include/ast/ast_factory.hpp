#pragma once

#include "ast/ast.hpp"

struct Ast_Factory {
    uint64_t node_count = 0;

    Location* location = NULL;

    template<typename T, typename ... Arguments>
    T* create (Location* loc, Arguments ... args) {
        this->node_count++;
        auto node = new T(args...);
        if (loc != NULL) {
            node->location = (*loc);
        }
        return node;
    }
};
