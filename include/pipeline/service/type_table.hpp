#pragma once

#include "ast/nodes.hpp"
#include "utils/ast_navigator.hpp"

#include "ast/types.hpp"

struct Type_Table {
    Build_Context* context = NULL;

    void init (Build_Context* c) { this->context = c; }

    void unique (Ast_Type** type_ptr) {
        auto type = (*type_ptr);

        if (type->guid > 0) return;

        switch (type->typedef_type) {
            case AST_TYPEDEF_STRUCT: {
                this->unique(reinterpret_cast<Ast_Struct_Type**>(type_ptr));
                break;
            }
            case AST_TYPEDEF_POINTER: {
                this->unique(reinterpret_cast<Ast_Pointer_Type**>(type_ptr));
                break;
            }
            case AST_TYPEDEF_FUNCTION: {
                this->unique(reinterpret_cast<Ast_Function_Type**>(type_ptr));
                break;
            }
            case AST_TYPEDEF_ARRAY: {
                this->unique(reinterpret_cast<Ast_Array_Type**>(type_ptr));
                break;
            }
            default: assert(false);
        }
    }

    void unique (Ast_Struct_Type** struct_ptr) {
        auto _struct = (*struct_ptr);

        printf("To unique struct type: '%s'\n", _struct->name);
    }

    void unique (Ast_Pointer_Type** ptr_type_ptr) {
        auto ptr_type = (*ptr_type_ptr);

        printf("To unique pointer type: '%s'\n", ptr_type->name);
    }

    void unique (Ast_Function_Type** func_type_ptr) {
        auto func_type = (*func_type_ptr);

        printf("To unique function type: '%s'\n", func_type->name);
    }

    void unique (Ast_Array_Type** array_type_ptr) {
        auto array_type = (*array_type_ptr);

        printf("To unique array type: '%s'\n", array_type->name);
    }
};
