#pragma once

#include "ast/ast.hpp"
#include "ast/types.hpp"

#define DECL_TYPE(type) this->statements.push_back(ast_make_declaration(type->name, type));

#define IS_OS(os_type) (os_get_type() == OS_TYPE_##os_type)

struct Internal_Scope : Ast_Scope {
    Internal_Scope () {
	    DECL_TYPE(Types::type_type);
	    DECL_TYPE(Types::type_void);
	    DECL_TYPE(Types::type_bool);
	    DECL_TYPE(Types::type_s8);
	    DECL_TYPE(Types::type_s16);
	    DECL_TYPE(Types::type_s32);
	    DECL_TYPE(Types::type_s64);
	    DECL_TYPE(Types::type_u8);
	    DECL_TYPE(Types::type_u16);
	    DECL_TYPE(Types::type_u32);
	    DECL_TYPE(Types::type_u64);
	    DECL_TYPE(Types::type_f32);
	    DECL_TYPE(Types::type_f64);
	    DECL_TYPE(Types::type_string);
	    DECL_TYPE(Types::type_any);

	    this->add_boolean("OS_WINDOWS", IS_OS(WINDOWS));
        this->add_boolean("OS_LINUX", IS_OS(LINUX));
        this->add_boolean("OS_MAC", IS_OS(MAC));
    }

    void add_boolean (const char* name, bool value) {
        this->add(ast_make_declaration(name, ast_make_literal(value)));
    }
};
