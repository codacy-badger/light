#pragma once

#include "ast/nodes.hpp"
#include "build_context.hpp"
#include "pipeline/service/type_table.hpp"

void internal_print_u64 (uint64_t number) {
    printf("%llu", number);
}

struct Internal_Scope : Ast_Scope {
    Location internal_location = Location("INTERNAL");

    Internal_Scope (Build_Context* context) {
        this->scope_flags |= SCOPE_FLAG_FULLY_PARSED;
        this->scope_flags |= SCOPE_FLAG_IMPORTS_RESOLVED;

        auto type_table = context->type_table;
	    this->add_type(type_table, type_table->type_namespace);
	    this->add_type(type_table, type_table->type_type);
	    this->add_type(type_table, type_table->type_void);
	    this->add_type(type_table, type_table->type_bool);
	    this->add_type(type_table, type_table->type_byte);
	    this->add_type(type_table, type_table->type_s8);
	    this->add_type(type_table, type_table->type_s16);
	    this->add_type(type_table, type_table->type_s32);
	    this->add_type(type_table, type_table->type_s64);
	    this->add_type(type_table, type_table->type_u8);
	    this->add_type(type_table, type_table->type_u16);
	    this->add_type(type_table, type_table->type_u32);
	    this->add_type(type_table, type_table->type_u64);
	    this->add_type(type_table, type_table->type_f32);
	    this->add_type(type_table, type_table->type_f64);
	    this->add_type(type_table, type_table->type_string);
	    this->add_type(type_table, type_table->type_any);

        auto target_os_type = context->target_os->type;
	    this->add_boolean(type_table, "OS_WINDOWS", target_os_type == OS_TYPE_WINDOWS);
        this->add_boolean(type_table, "OS_LINUX",   target_os_type == OS_TYPE_LINUX);
        this->add_boolean(type_table, "OS_MAC",     target_os_type == OS_TYPE_MAC);
        this->add_boolean(type_table, "DEBUG",      true);

        auto func = new Ast_Function("print_u64");

        func->arg_scope = new Ast_Scope();
        func->arg_scope->statements.push_back(new Ast_Declaration(NULL, type_table->type_u64));
        
        func->ret_scope = new Ast_Scope();
        func->ret_scope->statements.push_back(new Ast_Declaration(NULL, type_table->type_void));

        func->foreign_function_pointer = (void*) internal_print_u64;
        auto func_type = context->type_table->build_function_type(func);
        this->add_function(func_type, func);
    }

    void add_function (Ast_Function_Type* func_type, Ast_Function* func) {
        auto decl = new Ast_Declaration(func->name, func_type, func);
        decl->stm_flags |= STM_FLAG_IDENTS_RESOLVED;
        decl->stm_flags |= STM_FLAG_STATIC_IFS_RESOLVED;
        func_type->location = this->internal_location;
        func->location = this->internal_location;
        decl->location = this->internal_location;
        decl->is_constant = true;
        this->add(decl);
    }

    void add_type (Type_Table* type_table, Ast_Struct_Type* struct_type) {
        struct_type->inferred_type = type_table->type_type;

        auto decl = new Ast_Declaration(struct_type->name, type_table->type_type, struct_type);
        decl->stm_flags |= STM_FLAG_IDENTS_RESOLVED;
        decl->stm_flags |= STM_FLAG_STATIC_IFS_RESOLVED;
        decl->location = this->internal_location;
        decl->is_constant = true;
        this->add(decl);
    }

    void add_boolean (Type_Table* type_table, const char* _name, bool value) {
        auto literal = new Ast_Literal(value);
        literal->location = this->internal_location;

        auto decl = new Ast_Declaration(_name, type_table->type_bool, literal);
        decl->stm_flags |= STM_FLAG_IDENTS_RESOLVED;
        decl->stm_flags |= STM_FLAG_STATIC_IFS_RESOLVED;
        decl->location = this->internal_location;
        decl->is_constant = true;
        this->add(decl);
    }
};
