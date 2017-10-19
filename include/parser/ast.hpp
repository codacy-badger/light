#pragma once

#include <map>
#include <vector>
#include <string>

#include "lexer/lexer.hpp"

struct Ast_Note;
struct Ast_Ident;
struct Ast_Function;
struct Ast_Expression;
struct Ast_Declaration;
struct Ast_Type_Instance;
struct Ast_Type_Definition;

using namespace std;

struct Ast {
	const char* filename = NULL;
	long line = 0, col = 0;
};

enum Ast_Statement_Type {
	AST_STATEMENT_UNDEFINED = 0,
	AST_STATEMENT_BLOCK,
	AST_STATEMENT_NOTE,
	AST_STATEMENT_DECLARATION,
	AST_STATEMENT_RETURN,
	AST_STATEMENT_IMPORT,
	AST_STATEMENT_EXPRESSION,
};

struct Ast_Statement : Ast {
	Ast_Statement_Type stm_type = AST_STATEMENT_UNDEFINED;

	vector<Ast_Note*> notes;
};

struct Ast_Note : Ast_Statement {
	const char* name = NULL;

	Ast_Note() { this->stm_type = AST_STATEMENT_NOTE; }
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

	void find_all_declarations (std::vector<Ast_Declaration*>* decls);
};

const int DECL_FLAG_CONSTANT = 0x1;

struct Ast_Declaration : Ast_Statement {
	Ast_Ident* identifier = NULL;
	Ast_Type_Instance* type = NULL;
	Ast_Expression* expression = NULL;

	int decl_flags = 0;

	Ast_Declaration() { this->stm_type = AST_STATEMENT_DECLARATION; }
};

struct Ast_Return : Ast_Statement {
	Ast_Expression* exp = NULL;

	Ast_Return() { this->stm_type = AST_STATEMENT_RETURN; }
};

const int IMPORT_INCLUDE_CONTENT = 0x1;
const int IMPORT_IS_NATIVE = 0x2;

struct Ast_Import : Ast_Statement {
	const char* filepath = NULL;

	int import_flags = 0;

	Ast_Import() { this->stm_type = AST_STATEMENT_IMPORT; }
};

enum Ast_Expression_Type {
	AST_EXPRESSION_UNDEFINED = 0,
	AST_EXPRESSION_TYPE_DEFINITION,
	AST_EXPRESSION_TYPE_INSTANCE,
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

struct Ast_Type_Definition : Ast_Expression {
	vector<Ast_Declaration*> attributes;

	Ast_Type_Definition() { this->exp_type = AST_EXPRESSION_TYPE_DEFINITION; }
};

enum Ast_Type_Inst_Type {
	AST_TYPE_INST_UNDEFINED = 0,
	AST_TYPE_INST_STRUCT,
	AST_TYPE_INST_TYPE,
	AST_TYPE_INST_POINTER,
	AST_TYPE_INST_FUNCTION,
};

struct Ast_Type_Instance : Ast_Expression {
	Ast_Type_Inst_Type type_inst_type = AST_TYPE_INST_UNDEFINED;

	Ast_Type_Instance() { this->exp_type = AST_EXPRESSION_TYPE_INSTANCE; }
};

struct Ast_Pointer_Type : Ast_Type_Instance {
	Ast_Type_Instance* base = NULL;

	Ast_Pointer_Type() { this->type_inst_type = AST_TYPE_INST_POINTER; }
};

struct Ast_Function_Type : Ast_Type_Instance {
	vector<Ast_Type_Instance*> parameters;
	Ast_Type_Instance* return_type = NULL;

	Ast_Function_Type() { this->type_inst_type = AST_TYPE_INST_FUNCTION; }
};

struct Ast_Struct_Type : Ast_Type_Instance {
	const char* name = NULL;

	Ast_Struct_Type(const char* name = NULL) {
		this->type_inst_type = AST_TYPE_INST_STRUCT;
		this->name = name;
	}
};

struct Ast_Function : Ast_Expression {
	const char* name = NULL;
	Ast_Function_Type* type = NULL;
	Ast_Block* scope = NULL;

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

struct Ast_Binary : Ast_Expression {
	static map<Token_Type, bool> isLeftAssociate;
	static map<Token_Type, short> precedence;

	Ast_Binary_Type binary_op = AST_BINARY_UNINITIALIZED;
	Ast_Expression* lhs = NULL;
	Ast_Expression* rhs = NULL;

	Ast_Binary (Token_Type tType) {
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

struct Ast_Unary : Ast_Expression {
	Ast_Unary_Type unary_op = AST_UNARY_UNINITIALIZED;
	Ast_Expression* exp = NULL;

	Ast_Unary (Token_Type tType) {
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
	const char* name = NULL;

	Ast_Ident () { this->exp_type = AST_EXPRESSION_IDENT; }

	bool operator ==(const Ast_Ident* other) const;
};

Ast_Ident* ast_make_ident (const char* name);
