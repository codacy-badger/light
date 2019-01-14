#pragma once

#include "ast/ast.hpp"
#include "ast/types.hpp"
#include "ast/ast_factory.hpp"

#define DECLARE_TYPE_INT(name) this->add_uint("TYPE_" #name, INTERNAL_TYPE_##name)

#define IS_OS(os_type) (os_get_type() == OS_TYPE_##os_type)

struct Internal_Scope : Ast_Scope {
    Location internal_location = Location("INTERNAL");

    Internal_Scope () {
	    this->add_type(Types::type_type);
	    this->add_type(Types::type_void);
	    this->add_type(Types::type_bool);
	    this->add_type(Types::type_s8);
	    this->add_type(Types::type_s16);
	    this->add_type(Types::type_s32);
	    this->add_type(Types::type_s64);
	    this->add_type(Types::type_u8);
	    this->add_type(Types::type_u16);
	    this->add_type(Types::type_u32);
	    this->add_type(Types::type_u64);
	    this->add_type(Types::type_f32);
	    this->add_type(Types::type_f64);
	    this->add_type(Types::type_string);
	    this->add_type(Types::type_any);

	    this->add_boolean("OS_WINDOWS", IS_OS(WINDOWS));
        this->add_boolean("OS_LINUX",   IS_OS(LINUX));
        this->add_boolean("OS_MAC",     IS_OS(MAC));

        DECLARE_TYPE_INT(VOID);
        DECLARE_TYPE_INT(BOOL);
        DECLARE_TYPE_INT(INTEGER);
        DECLARE_TYPE_INT(DECIMAL);
        DECLARE_TYPE_INT(STRING);
        DECLARE_TYPE_INT(ANY);

        for (auto stm : this->statements) {
            stm->location = this->internal_location;
        }
    }

    void add_type (Ast_Type_Instance* type) {
        type->inferred_type = Types::type_type;
        this->add(Ast_Factory::declaration(type->name, type, NULL));
    }

    void add_boolean (const char* name, bool value) {
        this->add(Ast_Factory::declaration(name, Ast_Factory::literal(value)));
    }

    void add_uint (const char* name, uint64_t value) {
        this->add(Ast_Factory::declaration(name, Ast_Factory::literal(value)));
    }
};
