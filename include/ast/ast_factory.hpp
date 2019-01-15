#pragma once

#include "ast/ast.hpp"
#include "ast/types.hpp"

struct Ast_Factory {
    uint64_t node_count = 0;

    template<typename T, typename ... Arguments>
    T* create (Location* loc, Arguments ... args) {
        this->node_count++;
        auto node = new T(args...);
        if (loc != NULL) {
            node->location = (*loc);
        }
        return node;
    }

    static Ast_Declaration* declaration (const char* name, Ast_Expression* value,
            Ast_Expression* type = NULL, bool is_const = true) {
        auto decl = new Ast_Declaration();
        decl->type = type ? type : value->inferred_type;
        decl->is_constant = is_const;
        decl->expression = value;
        decl->name = name;
        return decl;
    }

    static Ast_Declaration* declaration (const char* name, Ast_Type_Instance* type_inst) {
        auto decl = new Ast_Declaration();
        decl->is_constant = true;
        decl->type = type_inst;
        decl->name = name;
        return decl;
    }

    static Ast_Pointer_Type* pointer_type (Ast_Expression* base_type) {
        auto pointer_type = new Ast_Pointer_Type(base_type);
        pointer_type->inferred_type = Types::type_type;
        return pointer_type;
    }

    static Ast_Array_Type* array_type (Ast_Expression* base_type, uint64_t length) {
        auto length_literal = Ast_Factory::literal(length);
        auto array_type = new Ast_Array_Type(base_type, length_literal);
        array_type->inferred_type = Types::type_type;
        return array_type;
    }

    static Ast_Literal* literal (const char* value) {
    	auto lit = new Ast_Literal();
        lit->inferred_type = Types::type_string;
    	lit->literal_type = AST_LITERAL_STRING;
    	lit->string_value = value;
    	return lit;
    }

    static Ast_Literal* literal (uint64_t value) {
    	auto lit = new Ast_Literal();
        lit->inferred_type = ast_get_smallest_type(value);
    	lit->literal_type = AST_LITERAL_UNSIGNED_INT;
    	lit->uint_value = value;
    	return lit;
    }

    static Ast_Literal* literal (bool value) {
    	auto lit = new Ast_Literal();
        lit->inferred_type = Types::type_bool;
    	lit->literal_type = AST_LITERAL_UNSIGNED_INT;
    	lit->uint_value = value;
    	return lit;
    }
};
