#pragma once

#include <vector>

#include "ast/ast_factory.hpp"
#include "lexer/lexer.hpp"
#include "pipeline/pipe.hpp"
#include "ast/notes.hpp"
#include "ast/ast.hpp"

using namespace std;

struct Parser {
	Notes* notes = new Notes();
	Lexer* lexer = NULL;

	Ast_Scope* current_scope = NULL;

	// for metrics
	size_t all_lines = 0;
	double time = 0;

	Ast_Scope* run (const char* filepath, Ast_Scope* parent = NULL);

	void setup (const char* filepath, Ast_Scope* parent = NULL);
	void teardown ();

	void push (Ast_Statement* stm);

	void scope (Ast_Scope* inner_scope);
	Ast_Note* note ();
	Ast_Directive* directive ();
	Ast_Statement* statement ();
	Ast_Declaration* declaration (Ast_Ident* ident = NULL);
	Ast_Declaration* declaration_or_type ();
	Ast_Expression* expression (Ast_Ident* initial = NULL, short minPrecedence = 1);
	void comma_separated_arguments (vector<Ast_Expression*>* arguments);
	Ast_Expression* _atom (Ast_Ident* initial = NULL);
	Ast_Expression* type_instance ();
	Ast_Function_Type* function_type ();
	Ast_Literal* literal ();
	Ast_Ident* ident ();

	void print_metrics(double total_time);
};
