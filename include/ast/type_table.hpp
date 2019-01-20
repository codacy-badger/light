#pragma once

#include "ast/ast.hpp"
#include "ast/types.hpp"

#include <vector>

struct Type_Table {
    std::vector<Ast_Type_Instance*> all_types;

    bool is_unique (Ast_Type_Instance* type) {
        for (auto _type : this->all_types) {
            if (type == _type) {
                return true;
            }
        }
        return false;
    }

    Ast_Type_Instance* find_unique (Ast_Type_Instance* type) {
        if (type->typedef_type == AST_TYPEDEF_STRUCT) {
            this->all_types.push_back(type);
            return type;
        } else if (type->name) {
            for (auto _type : this->all_types) {
                if (strcmp(type->name, _type->name) == 0) {
                    return _type;
                }
            }
        } else {
            for (auto _type : this->all_types) {
                if (Types::equal(type, _type)) {
                    return _type;
                }
            }
        }

        // @INFO we couldn't find the type in the global type table, so we
        // have to add the type as the unique instance
        this->all_types.push_back(type);
        return type;
    }
};
