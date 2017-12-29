#pragma once

#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <map>

#include "lexer/lexer.hpp"
#include "parser/pipes.hpp"
#include "parser/ast.hpp"

using namespace std;

struct Compiler;

struct Parser : Pipe {
	Lexer* lexer = NULL;
	Ast_Block* current_block = NULL;

	Parser (const char* param);

	void expected (const char* expect, const char* after);

	Ast_Block* top_level_block ();
	void block (Ast_Block* inner_block);
	Ast_Note* note ();
	Ast_Statement* statement ();
	Ast_Declaration* declaration (Ast_Ident* ident = NULL);
	Ast_Expression* expression (Ast_Ident* initial = NULL, short minPrecedence = 1);
	Ast_Comma_Separated_Arguments* comma_separated_arguments ();
	Ast_Expression* _atom (Ast_Ident* initial = NULL);
	Ast_Expression* type_definition ();
	Ast_Function_Type* function_type ();
	Ast_Function_Call* call (Ast_Expression* callee = NULL);
	Ast_Literal* literal ();
	Ast_Ident* ident (const char* name = NULL);
};
