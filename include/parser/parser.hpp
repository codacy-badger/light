#pragma once

#include <stdlib.h>
#include <vector>
#include <string>
#include <map>

#include "lexer/lexer.hpp"
#include "parser/pipes.hpp"
#include "parser/ast.hpp"

#define MAX_PATH_LENGTH 260

using namespace std;

struct Compiler;

struct Parser : Pipe {
	Lexer* lexer = NULL;
	char current_path[MAX_PATH_LENGTH];
	Ast_Block* current_block = NULL;

	vector<Ast_Note*> global_notes;
	vector<Ast_Note*> notes;

	Ast_Block* run (const char* filepath, Ast_Block* parent = NULL);
	void add (Ast_Statement* stm, Ast_Block* block = NULL);

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
};
