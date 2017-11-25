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

struct Light_Compiler;

struct Parser : Pipe {
	Light_Compiler* compiler = NULL;
	Lexer* lexer = NULL;
	Ast_Block* current_block = NULL;

	Parser (Light_Compiler* compiler, const char* param);

	void expected (const char* expect, const char* after);

	Ast_Block* top_level_block ();
	void block (Ast_Block* insert_block);
	Ast_Note* note ();
	Ast_Statement* statement ();
	Ast_Statement* statement_if ();
	Ast_Declaration* declaration (Ast_Ident* ident = NULL);
	Ast_Expression* expression (Ast_Ident* initial = NULL, short minPrecedence = 1);
	Ast_Comma_Separated_Arguments* comma_separated_arguments (Ast_Expression* exp = NULL);
	Ast_Expression* _atom (Ast_Ident* initial = NULL);
	Ast_Type_Definition* type_definition ();
	Ast_Function* function (Ast_Function_Type* fn_type);
	Ast_Function_Type* function_type ();
	Ast_Function_Call* call (Ast_Expression* callee);
	Ast_Literal* literal ();
	Ast_Ident* ident (const char* name = NULL);
};
