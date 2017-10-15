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

#define CHECK_TYPE(T, S) if(this->lexer->isNextType(TOKEN_TYPE_##T))	\
		this->lexer->skip(1);											\
	else expected("<token>", S)

#define AST_NEW(T, ...) setASTLocation(lexer, new T(__VA_ARGS__))
template <typename T>
T* setASTLocation (Lexer* lexer, T* node) {
	node->filename = lexer->buffer->source;
	node->line = lexer->buffer->line;
	node->col = lexer->buffer->col;
	return node;
}

template <typename T, typename O>
T* cast2 (O* obj) {
	return reinterpret_cast<T*>(obj);
}

struct Parser : Pipe {
	Lexer* lexer;
	Ast_Block* currentScope;

	template <typename LexerParam>
	Parser (LexerParam param) {
		this->lexer = new Lexer(param);
		this->currentScope = AST_NEW(Ast_Block, "<global>");
	}

	void expected (const char* expect, const char* after);

	bool block ();
	Ast_Statement* statement ();
	Ast_Type_Definition* type ();
	Ast_Struct_Type* structType (string name);
	void _typeBody (vector<Ast_Declaration*>* attributes);
	Ast_Function* function ();
	Ast_Function_Type* _functionType ();
	void _functionParameters (vector<Ast_Declaration*>* parameters);
	Ast_Type_Definition* _typeInstance ();
	Ast_Return* returnStm ();
	Ast_Expression* expression (short minPrecedence = 1);
	Ast_Expression* _atom ();
	Ast_Literal* literal ();
	Ast_Function_Call* call (Ast_Expression* callee);
	Ast_Ident* ident ();

	void scopePush (string name);
	void scopePop ();
};
