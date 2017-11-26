#pragma once

#include "lexer/lexer.hpp"

#include <vector>

struct Ast_Function;
struct Ast_Expression;
struct Ast_Declaration;
struct Ast_Struct_Type;
struct Ast_Type_Definition;
struct Ast_Comma_Separated_Arguments;

struct Instruction;

using namespace std;

struct Ast {
	const char* filename = NULL;
	long line = 0, col = 0;
};

void ast_copy_location_info (Ast* destination, Ast* source);

struct Ast_Note : Ast {
	const char* name = NULL;
	Ast_Comma_Separated_Arguments* arguments = NULL;
};

enum Ast_Statement_Type {
	AST_STATEMENT_UNDEFINED = 0,
	AST_STATEMENT_BLOCK,
	AST_STATEMENT_IF,
	AST_STATEMENT_WHILE,
	AST_STATEMENT_DECLARATION,
	AST_STATEMENT_RETURN,
	AST_STATEMENT_IMPORT,
	AST_STATEMENT_EXPRESSION,
};

struct Ast_Statement : Ast {
	Ast_Statement_Type stm_type = AST_STATEMENT_UNDEFINED;

	vector<Ast_Note*> notes;
};

struct Ast_Block : Ast_Statement {
	const char* name = NULL;
	vector<Ast_Statement*> list;

	Ast_Block* parent = NULL;
	Ast_Function* scope_of = NULL;

	Ast_Block (Ast_Block* parent = NULL) {
		this->stm_type = AST_STATEMENT_BLOCK;
		this->parent = parent;
	}

	bool is_global();
	void find_declarations (std::vector<Ast_Declaration*>* decls, bool recurse = true);
	Ast_Declaration* find_declaration (const char* name, bool recurse = true);
	Ast_Type_Definition* find_type_definition (const char* name, bool recurse = true);
	Ast_Function* find_function (bool recurse = true);
};

struct Ast_If : Ast_Statement {
	Ast_Expression* condition = NULL;
	Ast_Statement* then_statement = NULL;
	Ast_Statement* else_statement = NULL;

	Ast_If () { this->stm_type = AST_STATEMENT_IF; }
};

struct Ast_While : Ast_Statement {
	Ast_Expression* condition = NULL;
	Ast_Statement* statement = NULL;

	Ast_While () { this->stm_type = AST_STATEMENT_WHILE; }
};

const uint8_t DECL_FLAG_CONSTANT = 0x1;

struct Ast_Declaration : Ast_Statement {
	const char* name = NULL;
	Ast_Expression* type = NULL;
	Ast_Expression* expression = NULL;

	uint8_t decl_flags = 0;

	Ast_Block* scope = NULL;

	// for bytecode
	size_t stack_offset = 0;

	// If struct property
	Ast_Struct_Type* _struct = NULL;
	uint16_t struct_index = 0;
	uint16_t struct_byte_offset = 0;

	Ast_Declaration() { this->stm_type = AST_STATEMENT_DECLARATION; }

	bool is_global();
};

struct Ast_Return : Ast_Statement {
	Ast_Expression* exp = NULL;
	Ast_Block* block = NULL;

	Ast_Return() { this->stm_type = AST_STATEMENT_RETURN; }
};

const uint8_t IMPORT_INCLUDE_CONTENT = 0x1;
const uint8_t IMPORT_IS_NATIVE 		 = 0x2;

struct Ast_Import : Ast_Statement {
	const char* filepath = NULL;

	uint8_t import_flags = 0;

	Ast_Import() { this->stm_type = AST_STATEMENT_IMPORT; }
};

enum Ast_Expression_Type {
	AST_EXPRESSION_UNDEFINED = 0,
	AST_EXPRESSION_COMMA_SEPARATED_ARGUMENTS,
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
	Ast_Type_Definition* inferred_type = NULL;

	Ast_Expression() { this->stm_type = AST_STATEMENT_EXPRESSION; }
};

struct Ast_Comma_Separated_Arguments : Ast_Expression {
	vector<Ast_Expression*> values;

	Ast_Comma_Separated_Arguments() { this->exp_type = AST_EXPRESSION_COMMA_SEPARATED_ARGUMENTS; }
};

enum Ast_Type_Definition_Type {
	AST_TYPEDEF_UNDEFINED = 0,
	AST_TYPEDEF_FUNCTION,
	AST_TYPEDEF_STRUCT,
	AST_TYPEDEF_POINTER,
};

#define AST_PTR_SIZE sizeof(void*)

struct Ast_Type_Definition : Ast_Expression {
	Ast_Type_Definition_Type typedef_type = AST_TYPEDEF_UNDEFINED;
	size_t byte_size = 0;

	Ast_Type_Definition() { this->exp_type = AST_EXPRESSION_TYPE_DEFINITION; }
};

struct Ast_Function_Type : Ast_Type_Definition {
	vector<Ast_Declaration*> parameter_decls;
	Ast_Expression* return_type = NULL;

	Ast_Function_Type() {
		this->typedef_type = AST_TYPEDEF_FUNCTION;
		this->byte_size = AST_PTR_SIZE;
	}
};

struct Ast_Struct_Type : Ast_Type_Definition {
	const char* name = NULL;
	vector<Ast_Declaration*> attributes;

	Ast_Struct_Type(const char* name = NULL, size_t byte_size = 0) {
		this->typedef_type = AST_TYPEDEF_STRUCT;
		this->byte_size = byte_size;
		this->name = name;
	}
};

struct Ast_Pointer_Type : Ast_Type_Definition {
	Ast_Expression* base = NULL;

	Ast_Pointer_Type() {
		this->typedef_type = AST_TYPEDEF_POINTER;
		this->byte_size = AST_PTR_SIZE;
	}
};

struct Ast_Function : Ast_Expression {
	const char* name = NULL;
	Ast_Function_Type* type = NULL;
	Ast_Block* scope = NULL;

	const char* foreign_module_name = NULL;
	const char* foreign_function_name = NULL;

	vector<Instruction*> bytecode;

	Ast_Function() { this->exp_type = AST_EXPRESSION_FUNCTION; }
};

enum Ast_Binary_Type {
	AST_BINARY_UNINITIALIZED,
	AST_BINARY_ASSIGN,
	AST_BINARY_ATTRIBUTE,
	AST_BINARY_SUBSCRIPT,

	AST_BINARY_LOGICAL_AND,
	AST_BINARY_LOGICAL_OR,

	AST_BINARY_ADD,
	AST_BINARY_SUB,
	AST_BINARY_MUL,
	AST_BINARY_DIV,

	AST_BINARY_BITWISE_AND,
	AST_BINARY_BITWISE_OR,
	AST_BINARY_BITWISE_XOR,
	AST_BINARY_BITWISE_RIGHT_SHIFT,
	AST_BINARY_BITWISE_LEFT_SHIFT,

	AST_BINARY_EQ,
	AST_BINARY_NEQ,
	AST_BINARY_LT,
	AST_BINARY_LTE,
	AST_BINARY_GT,
	AST_BINARY_GTE,
};

struct Ast_Binary : Ast_Expression {
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
	AST_UNARY_NOT,
	AST_UNARY_NEGATE,
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
	Ast_Comma_Separated_Arguments* args = NULL;

	Ast_Function_Call() { this->exp_type = AST_EXPRESSION_CALL; }
};

enum Ast_Literal_Type {
	AST_LITERAL_UNINITIALIZED,
	AST_LITERAL_SIGNED_INT,
	AST_LITERAL_UNSIGNED_INT,
	AST_LITERAL_DECIMAL,
	AST_LITERAL_STRING,
};

struct Ast_Literal : Ast_Expression {
	Ast_Literal_Type literal_type = AST_LITERAL_UNINITIALIZED;

	union {
		int64_t  	int_value;
		uint64_t  	uint_value;
		double 		decimal_value;
		char* 		string_value;
	};

	// for bytecode
	size_t data_offset = 0;

	Ast_Literal () { this->exp_type = AST_EXPRESSION_LITERAL; }
};

struct Ast_Ident : Ast_Expression {
	const char* name = NULL;
	Ast_Declaration* declaration = NULL;

	Ast_Ident () { this->exp_type = AST_EXPRESSION_IDENT; }

	bool operator ==(const Ast_Ident* other) const;
};

Ast_Ident* ast_make_ident (const char* name);
Ast_Declaration* ast_make_declaration (const char* name, Ast_Expression* exp, bool is_const = true);
