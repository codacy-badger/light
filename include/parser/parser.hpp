#pragma once

#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "lexer/lexer.hpp"
#include "parser/pipes.hpp"
#include "parser/ast.hpp"

using namespace std;

struct Parser : Pipe {
	Lexer* lexer;
	Ast_Block* currentScope;

	Parser (const char* param);

	void expected (const char* expect, const char* after);

	bool block ();
	Ast_Statement* statement ();
	Ast_Declaration* declaration ();
	Ast_Type_Instance* type_instance ();
	Ast_Struct_Type* structType (string name);
	Ast_Expression* expression (Ast_Ident* initial = NULL, short minPrecedence = 1);
	Ast_Expression* _atom (Ast_Ident* initial = NULL);
	Ast_Type_Definition* type_definition ();
	Ast_Function* function ();
	Ast_Function_Type* _functionType ();
	void _functionParameters (vector<Ast_Declaration*>* parameters);
	Ast_Function_Call* call (Ast_Expression* callee);
	Ast_Literal* literal ();
	Ast_Ident* ident ();

	void scopePush (string name);
	void scopePop ();
};
