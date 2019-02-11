#pragma once

#include "ast/nodes.hpp"
#include "ast/types.hpp"
#include "ast/factory.hpp"

#define DECLARE_TYPE_INT(name) this->add_uint("TYPE_" #name, INTERNAL_TYPE_##name)

struct Internal_Scope : Ast_Scope {
    Location internal_location = Location("INTERNAL");

    Internal_Scope (Arch* target_arch, OS* target_os) {
        this->scope_flags |= SCOPE_FLAG_FULLY_PARSED;
        this->scope_flags |= SCOPE_FLAG_SYMBOLS_RESOLVED;

        Types::type_usize->byte_size = target_arch->register_size;

	    this->add_type(Types::type_type);
	    this->add_type(Types::type_void);
	    this->add_type(Types::type_bool);
	    this->add_type(Types::type_byte);
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
	    this->add_type(Types::type_usize);
	    this->add_type(Types::type_string);
	    this->add_type(Types::type_any);

	    this->add_boolean("OS_WINDOWS", target_os->type == OS_TYPE_WINDOWS);
        this->add_boolean("OS_LINUX",   target_os->type == OS_TYPE_LINUX);
        this->add_boolean("OS_MAC",     target_os->type == OS_TYPE_MAC);

        DECLARE_TYPE_INT(VOID);
        DECLARE_TYPE_INT(BOOL);
        DECLARE_TYPE_INT(INTEGER);
        DECLARE_TYPE_INT(DECIMAL);
        DECLARE_TYPE_INT(STRING);
        DECLARE_TYPE_INT(ANY);
    }

    void add_type (Ast_Type* type) {
        type->inferred_type = Types::type_type;
        this->add(Ast_Factory::declaration(internal_location, type->name, Types::type_type, type));
    }

    void add_boolean (const char* name, bool value) {
        auto literal = Ast_Factory::literal(internal_location, value);
        this->add(Ast_Factory::declaration(internal_location, name, literal->inferred_type, literal));
    }

    void add_uint (const char* name, uint64_t value) {
        auto literal = Ast_Factory::literal(internal_location, value);
        this->add(Ast_Factory::declaration(internal_location, name, literal->inferred_type, literal));
    }
};
