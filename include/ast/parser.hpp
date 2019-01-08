#pragma once

#include <vector>

#include "ast/ast_factory.hpp"
#include "lexer/lexer.hpp"
#include "pipeline/pipe.hpp"
#include "ast/ast.hpp"
#include "ast/types.hpp"

#include "phase/async_phase.hpp"

using namespace std;

#define DECL_TYPE(scope, type) scope->statements.push_back(ast_make_declaration(type->name, type));

#define IS_WINDOWS_LITERAL ast_make_literal(os_get_type() == OS_TYPE_WINDOWS)
#define IS_LINUX_LITERAL ast_make_literal(os_get_type() == OS_TYPE_LINUX)
#define IS_MAC_LITERAL ast_make_literal(os_get_type() == OS_TYPE_MAC)

struct Parser : Async_Phase {
	// TODO: merge this attribute with current_scope
	Ast_Scope* internal_scope = new Ast_Scope();
	Ast_Factory* factory = new Ast_Factory();
	Ast_Scope* current_scope = NULL;

	vector<Token*>* tokens = NULL;
	size_t index;

	// for metrics
	size_t all_lines = 0;
	double total_time = 0;

	Parser () : Async_Phase("Parser", CE_MODULE_LEXED) {
	    DECL_TYPE(this->internal_scope, Types::type_type);
	    DECL_TYPE(this->internal_scope, Types::type_void);
	    DECL_TYPE(this->internal_scope, Types::type_bool);
	    DECL_TYPE(this->internal_scope, Types::type_s8);
	    DECL_TYPE(this->internal_scope, Types::type_s16);
	    DECL_TYPE(this->internal_scope, Types::type_s32);
	    DECL_TYPE(this->internal_scope, Types::type_s64);
	    DECL_TYPE(this->internal_scope, Types::type_u8);
	    DECL_TYPE(this->internal_scope, Types::type_u16);
	    DECL_TYPE(this->internal_scope, Types::type_u32);
	    DECL_TYPE(this->internal_scope, Types::type_u64);
	    DECL_TYPE(this->internal_scope, Types::type_f32);
	    DECL_TYPE(this->internal_scope, Types::type_f64);
	    DECL_TYPE(this->internal_scope, Types::type_string);
	    DECL_TYPE(this->internal_scope, Types::type_any);

	    this->internal_scope->add(ast_make_declaration("OS_WINDOWS", IS_WINDOWS_LITERAL));
	    this->internal_scope->add(ast_make_declaration("OS_LINUX", IS_LINUX_LITERAL));
	    this->internal_scope->add(ast_make_declaration("OS_MAC", IS_MAC_LITERAL));
	}

    void handle (void* data) {
		auto module = reinterpret_cast<Module*>(data);

		this->tokens = module->tokens;
		module->global_scope = this->build_ast();

		Events::trigger(CE_MODULE_PARSED, module);
    }

	Ast_Scope* build_ast ();

	Ast_Scope* scope (Ast_Scope* inner_scope = NULL);
	Ast_Directive* directive ();
	Ast_Statement* statement ();
	Ast_If* _if ();
	Ast_Scope* scoped_statement ();
	Ast_Declaration* declaration ();
	Ast_Declaration* declaration_or_type ();
	Ast_Expression* expression (short minPrecedence = 1);
	Ast_Arguments* arguments ();
	Ast_Expression* atom ();
	Ast_Expression* type_instance ();
	Ast_Function_Type* function_type ();
	Ast_Literal* literal ();
	Ast_Literal* string_literal ();
	Ast_Ident* ident ();

	const char* escape_string (const char* original, size_t length);
	const char* escaped_string ();

	Token* peek (size_t offset = 0);
	Token* next ();
	bool is_next (Token_Type type);
	void skip (size_t offset = 1);
	bool try_skip (Token_Type type);
	void expect (Token_Type type);
	void report_expected (const char* name);

	void print_extra_metrics();
};
