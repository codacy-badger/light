#pragma once

#include "ast/ast.hpp"
#include "ast/types.hpp"

struct Ast_Factory {
    template<typename T, typename ... Arguments>
    static T* create (Lexer* lexer, Arguments ... args) {
        auto node = new T(args...);
        node->location.filename = lexer->scanner.absolute_path;
        node->location.line = lexer->scanner.current_line;
        return node;
    }

    static Ast_Declaration* declaration (Location location, const char* name,
            Ast_Expression* type = NULL, Ast_Expression* value = NULL, bool is_const = true) {
        auto decl = new Ast_Declaration();
        decl->location = location;
        decl->type = type ? type : value->inferred_type;
        decl->is_constant = is_const;
        decl->expression = value;
        decl->name = name;
        return decl;
    }

    static Ast_Pointer_Type* pointer_type (Ast_Expression* base_type) {
        auto pointer_type = new Ast_Pointer_Type(base_type);
        pointer_type->inferred_type = Types::type_type;
        pointer_type->location = base_type->location;
        return pointer_type;
    }

    static Ast_Array_Type* array_type (Ast_Expression* base_type, uint64_t length) {
        auto length_literal = Ast_Factory::literal(base_type->location, length);
        auto array_type = new Ast_Array_Type(base_type, length_literal);
        array_type->inferred_type = Types::type_type;
        array_type->location = base_type->location;
        return array_type;
    }

    static Ast_Binary* attr (Ast_Expression* exp, const char* attr_name) {
        assert(exp->inferred_type->typedef_type == AST_TYPEDEF_STRUCT);

        auto _struct = static_cast<Ast_Struct_Type*>(exp->inferred_type);
        return Ast_Factory::attr(exp, attr_name, _struct);
    }

    static Ast_Binary* attr (Ast_Expression* exp, const char* attr_name, Ast_Struct_Type* type) {
        auto attr_decl = type->find_attribute(attr_name);
        assert(attr_decl != NULL);

        assert(attr_decl->type->exp_type == AST_EXPRESSION_TYPE_INSTANCE);
        auto inferred_type = static_cast<Ast_Type_Instance*>(attr_decl->type);

        return Ast_Factory::attr(exp, attr_name, inferred_type, attr_decl);
    }

    static Ast_Binary* attr (Ast_Expression* exp, const char* attr_name, Ast_Type_Instance* inferred_type, Ast_Declaration* attr_decl) {
        assert(attr_decl->type->exp_type == AST_EXPRESSION_TYPE_INSTANCE);
        auto attr_inferred_type = static_cast<Ast_Type_Instance*>(attr_decl->type);

        auto binop = new Ast_Binary(AST_BINARY_ATTRIBUTE);
        binop->inferred_type = inferred_type;
        binop->location = exp->location;
        binop->lhs = exp;
        binop->rhs = Ast_Factory::ident(exp->location, attr_name, attr_decl, attr_inferred_type);
    	return binop;
    }

    static Ast_Binary* assign (Ast_Expression* exp1, Ast_Expression* exp2, Ast_Type_Instance* inferred_type = NULL) {
    	auto binop = new Ast_Binary(AST_BINARY_ASSIGN);
        binop->inferred_type = inferred_type;
        binop->location = exp1->location;
        binop->lhs = exp1;
        binop->rhs = exp2;
    	return binop;
    }

    static Ast_Unary* ref (Ast_Expression* exp, Ast_Type_Instance* inferred_type = NULL) {
    	auto unop = new Ast_Unary(AST_UNARY_REFERENCE);
        unop->inferred_type = inferred_type;
        unop->location = exp->location;
        unop->exp = exp;
    	return unop;
    }

    static Ast_Ident* ident (Location location, const char* name,
            Ast_Declaration* decl = NULL, Ast_Type_Instance* inferred_type = NULL) {
    	auto ident = new Ast_Ident();
        ident->inferred_type = inferred_type;
        ident->location = location;
        ident->declaration = decl;
    	ident->name = name;
    	return ident;
    }

    static Ast_Array_Type* literal_string_type (const char* text) {
        return Ast_Factory::array_type(Types::type_byte, strlen(text));
    }

    static Ast_Literal* literal_array_length (Location location, Ast_Type_Instance* type) {
        assert(type->typedef_type == AST_TYPEDEF_ARRAY);
        auto arr_type = static_cast<Ast_Array_Type*>(type);
        return Ast_Factory::literal_array_length(location, arr_type);
    }

    static Ast_Literal* literal_array_length (Location location, Ast_Array_Type* arr_type) {
        assert(arr_type->length_uint > 0);
        return Ast_Factory::literal(location, arr_type->length_uint);
    }

    static Ast_Literal* literal (Location location, const char* value) {
    	auto lit = new Ast_Literal();
        lit->location = location;
    	lit->literal_type = AST_LITERAL_STRING;
    	lit->string_value = value;
    	return lit;
    }

    static Ast_Literal* literal (Location location, uint64_t value) {
    	auto lit = new Ast_Literal();
        lit->location = location;
        lit->inferred_type = ast_get_smallest_type(value);
    	lit->literal_type = AST_LITERAL_UNSIGNED_INT;
    	lit->uint_value = value;
    	return lit;
    }

    static Ast_Literal* literal (Location location, bool value) {
    	auto lit = new Ast_Literal();
        lit->location = location;
        lit->inferred_type = Types::type_bool;
    	lit->literal_type = AST_LITERAL_UNSIGNED_INT;
    	lit->uint_value = value;
    	return lit;
    }
};
