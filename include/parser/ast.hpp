#pragma once

#include <map>
#include <vector>
#include <string>

#include "lexer/lexer.hpp"

struct Ast_Function;
struct Ast_Expression;
struct Ast_Type_Definition;

using namespace std;

struct AST {
	const char* filename;
	long line, col;
};

enum Ast_Statement_Type {
	AST_STATEMENT_UNDEFINED = 0,
	AST_STATEMENT_BLOCK,
	AST_STATEMENT_DECLARATION,
	AST_STATEMENT_RETURN,
	AST_STATEMENT_EXPRESSION,
};

struct Ast_Statement : AST {
	Ast_Statement_Type stm_type = AST_STATEMENT_UNDEFINED;
};

struct Ast_Block : Ast_Statement {
	string name;
	vector<Ast_Statement*> list;

	Ast_Block* parent = NULL;
	map<string, Ast_Expression*> symbols;

	Ast_Block (Ast_Block* parent = NULL) {
		this->stm_type = AST_STATEMENT_BLOCK;
		this->parent = parent;
	}
};

struct Ast_Declaration : Ast_Statement {
	string name;
	Ast_Type_Definition* type;
	Ast_Expression* expression;

	Ast_Declaration() { this->stm_type = AST_STATEMENT_DECLARATION; }
};

struct Ast_Return : Ast_Statement {
	Ast_Expression* exp = NULL;

	Ast_Return() { this->stm_type = AST_STATEMENT_RETURN; }
};

enum Ast_Expression_Type {
	AST_EXPRESSION_UNDEFINED = 0,
	AST_EXPRESSION_TYPE_DEFINITION,
	AST_EXPRESSION_FUNCTION,
	AST_EXPRESSION_BINARY,
	AST_EXPRESSION_UNARY,
	AST_EXPRESSION_CALL,
	AST_EXPRESSION_IDENT,
	AST_EXPRESSION_LITERAL,
};

struct Ast_Expression : Ast_Statement {
	Ast_Expression_Type exp_type = AST_EXPRESSION_UNDEFINED;

	Ast_Expression() { this->stm_type = AST_STATEMENT_EXPRESSION; }
};

enum Ast_Type_Def_Type {
	AST_TYPE_DEF_UNDEFINED = 0,
	AST_TYPE_DEF_STRUCT,
	AST_TYPE_DEF_POINTER,
	AST_TYPE_DEF_FUNCTION,
	AST_TYPE_DEF_PRIMITIVE,
};

struct Ast_Type_Definition : Ast_Expression {
	Ast_Type_Def_Type type_def_type = AST_TYPE_DEF_UNDEFINED;

	vector<Ast_Declaration*> attributes;

	Ast_Type_Definition() { this->exp_type = AST_EXPRESSION_TYPE_DEFINITION; }
};

struct Ast_Pointer_Type : Ast_Type_Definition {
	Ast_Type_Definition* base = NULL;

	Ast_Pointer_Type() { this->type_def_type = AST_TYPE_DEF_POINTER; }
};

struct Ast_Function_Type : Ast_Type_Definition {
	vector<Ast_Declaration*> parameters;
	Ast_Type_Definition* retType = NULL;

	Ast_Function_Type() { this->type_def_type = AST_TYPE_DEF_FUNCTION; }
};

struct Ast_Struct_Type : Ast_Type_Definition {
	string name;

	Ast_Struct_Type(string name = "") {
		this->type_def_type = AST_TYPE_DEF_STRUCT;
		this->name = name;
	}
};

struct Ast_Primitive_Type : Ast_Type_Definition {
	string name;

	Ast_Primitive_Type (string name) {
		this->type_def_type = AST_TYPE_DEF_PRIMITIVE;
		this->name = name;
	}

	static Ast_Type_Definition* _void;
	static Ast_Type_Definition* _i1;
	static Ast_Type_Definition* _i8;
	static Ast_Type_Definition* _i16;
	static Ast_Type_Definition* _i32;
	static Ast_Type_Definition* _i64;
	static Ast_Type_Definition* _i128;
};

struct Ast_Function : Ast_Expression {
	string name;
	Ast_Function_Type* type = NULL;
	Ast_Statement* stm = NULL;

	Ast_Function() { this->exp_type = AST_EXPRESSION_FUNCTION; }
};

enum Ast_Binary_Type {
	AST_BINARY_UNINITIALIZED,
	AST_BINARY_ASSIGN,
	AST_BINARY_ATTRIBUTE,
	AST_BINARY_SUBSCRIPT,
	AST_BINARY_ADD,
	AST_BINARY_SUB,
	AST_BINARY_MUL,
	AST_BINARY_DIV,
};

struct AST_Binary : Ast_Expression {
	static map<Token_Type, bool> isLeftAssociate;
	static map<Token_Type, short> precedence;

	Ast_Binary_Type binary_op = AST_BINARY_UNINITIALIZED;
	Ast_Expression* lhs = NULL;
	Ast_Expression* rhs = NULL;

	AST_Binary (Token_Type tType) {
		this->exp_type = AST_EXPRESSION_BINARY;
		this->setOP(tType);
	}

	void setOP (Token_Type tType);

	Ast_Binary_Type typeToOP (Token_Type tType);
	static short getPrecedence (Token_Type opToken);
	static bool getLeftAssociativity (Token_Type opToken);
};

enum Ast_Unary_Type {
	AST_UNARY_UNINITIALIZED,
	AST_UNARY_DEREFERENCE,
	AST_UNARY_REFERENCE,
	AST_UNARY_NEGATE_EXPRESSION,
	AST_UNARY_NEGATE_NUMBER,
};

struct AST_Unary : Ast_Expression {
	Ast_Unary_Type unary_op = AST_UNARY_UNINITIALIZED;
	Ast_Expression* exp = NULL;

	AST_Unary (Token_Type tType) {
		this->exp_type = AST_EXPRESSION_UNARY;
		this->setOP(tType);
	}

	void setOP (Token_Type tType);
	Ast_Unary_Type typeToOP (Token_Type tType);
};

struct Ast_Function_Call : Ast_Expression {
	Ast_Expression* fn;
	vector<Ast_Expression*> parameters;

	Ast_Function_Call() { this->exp_type = AST_EXPRESSION_CALL; }
};

enum Ast_Literal_Type {
	AST_LITERAL_UNINITIALIZED,
	AST_LITERAL_INTEGER,
	AST_LITERAL_DECIMAL,
	AST_LITERAL_STRING,
};

struct Ast_Literal : Ast_Expression {
	Ast_Literal_Type literal_type = AST_LITERAL_UNINITIALIZED;
	union {
		int64_t integer_value;
		double decimal_value;
		char* string_value;
	};

	Ast_Literal () { this->exp_type = AST_EXPRESSION_LITERAL; }
};

struct Ast_Ident : Ast_Expression {
	string name = "";

	Ast_Ident () { this->exp_type = AST_EXPRESSION_IDENT; }
};
