#pragma once

#include <vector>

#include "ast/ast_factory.hpp"
#include "lexer/lexer.hpp"
#include "pipeline/pipe.hpp"
#include "ast/ast.hpp"

using namespace std;

struct Parser {
	Ast_Scope* current_scope = NULL;
	Lexer* lexer = NULL;

	// for metrics
	size_t all_lines = 0;
	double time = 0;

	Ast_Scope* run (Lexer* lexer, Ast_Scope* parent = NULL);

	Ast_Scope* scope (Ast_Scope* inner_scope = NULL);
	const char* note ();
	Ast_Directive* directive ();
	Ast_Statement* statement ();
	Ast_Scope* scoped_statement ();
	Ast_Declaration* declaration (Ast_Ident* ident = NULL);
	Ast_Declaration* declaration_or_type ();
	Ast_Expression* expression (Ast_Ident* initial = NULL, short minPrecedence = 1);
	Ast_Arguments* arguments ();
	Ast_Expression* _atom (Ast_Ident* initial = NULL);
	Ast_Expression* type_instance ();
	Ast_Function_Type* function_type ();
	Ast_Literal* literal ();
	Ast_Literal* string_literal ();
	Ast_Ident* ident ();

	void print_metrics(double total_time);
};
