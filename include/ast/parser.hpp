#pragma once

#include <vector>

#include "ast/ast_factory.hpp"
#include "lexer/lexer.hpp"
#include "pipeline/pipe.hpp"
#include "ast/ast.hpp"

using namespace std;

struct Parser : Pipe {
	Ast_Factory* factory = new Ast_Factory();
	Lexer* lexer = NULL;

	char last_path[MAX_PATH_LENGTH];
	Ast_Block* current_block = NULL;

	vector<Ast_Note*> global_notes;
	vector<Ast_Note*> notes;

	// for metrics
	size_t all_lines = 0;

	Parser () { this->pipe_name = "Parser & Lexer"; }

	Lexer* setup (const char* filepath, Ast_Block* parent = NULL);
	void teardown ();

	void block (Ast_Block* inner_block);
	Ast_Note* note ();
	Ast_Statement* statement ();
	Ast_Declaration* declaration (Ast_Ident* ident = NULL);
	Ast_Declaration* declaration_or_type ();
	Ast_Expression* expression (Ast_Ident* initial = NULL, short minPrecedence = 1);
	void comma_separated_arguments (vector<Ast_Expression*>* arguments);
	Ast_Expression* _atom (Ast_Ident* initial = NULL);
	Ast_Expression* type_instance ();
	Ast_Function_Type* function_type ();
	Ast_Literal* literal ();
	Ast_Ident* ident (const char* name = NULL);

	virtual void print_pipe_metrics();
};
