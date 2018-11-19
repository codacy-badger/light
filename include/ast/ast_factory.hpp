#pragma once

#include "ast/ast.hpp"

struct Ast_Factory {
    static uint64_t node_count;

    template<typename T, typename ... Arguments>
    static T* create_node (Location* loc = NULL, Arguments ... args) {
        Ast_Factory::node_count++;
        auto node = new T(args...);
        if (loc != NULL) {
            memcpy(&node->location, loc, sizeof(Location));
        }
        return node;
    }

    static void delete_node (Ast* node) {
        Ast_Factory::node_count--;
        delete node;
    }
};
