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
		this->currentScope->add("void", Ast_Primitive_Type::_void);
		this->currentScope->add("i1",   Ast_Primitive_Type::_i1);
		this->currentScope->add("i8",   Ast_Primitive_Type::_i8);
		this->currentScope->add("i16",  Ast_Primitive_Type::_i16);
		this->currentScope->add("i32",  Ast_Primitive_Type::_i32);
		this->currentScope->add("i64",  Ast_Primitive_Type::_i64);
		this->currentScope->add("i128", Ast_Primitive_Type::_i128);
	}

	void expected (const char* expect, const char* after);

	bool block ();
	Ast_Statement* statement ();
	Ast_Type_Definition* type ();
	Ast_Struct_Type* structType (string name);
	Ast_Statement* _typeBody ();
	Ast_Function* function ();
	Ast_Function_Type* _functionType ();
	void _functionParameters (vector<Ast_Variable*>* output);
	Ast_Variable* var_def ();
	Ast_Variable* _var_def ();
	Ast_Type_Definition* _typeInstance ();
	Ast_Return* returnStm ();
	Ast_Expression* expression (short minPrecedence = 1);
	Ast_Expression* _atom ();
	Ast_Literal* literal ();
	Ast_Function_Call* call (Ast_Expression* callee);
	Ast_Expression* variable ();

	void scopePush (string name);
	void scopePop ();
};
