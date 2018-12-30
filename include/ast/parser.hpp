#pragma once

#include <vector>

#include "ast/ast_factory.hpp"
#include "lexer/lexer.hpp"
#include "pipeline/pipe.hpp"
#include "ast/ast.hpp"

using namespace std;

struct Parser {
	Ast_Factory* factory = new Ast_Factory();
	Ast_Scope* current_scope = NULL;

	static Token* eof;
	vector<Token*> tokens;
	size_t index;

	// for metrics
	size_t all_lines = 0;
	double total_time = 0;

	Ast_Scope* build_ast (Ast_Scope* parent);

	Token* peek (size_t offset = 0);
	Token* next ();
	bool is_next (Token_Type type);
	void skip (size_t offset = 1);
	bool try_skip (Token_Type type);
	bool expect (Token_Type type);

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
