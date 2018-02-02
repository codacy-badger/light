#pragma once

#include "lexer/lexer.hpp"

#include <vector>

struct Ast_Function;
struct Ast_Expression;
struct Ast_Declaration;
struct Ast_Struct_Type;
struct Ast_Type_Instance;

struct Instruction;

using namespace std;

bool ast_is_poly (Ast_Expression* exp);

struct Ast {
	Location location;
};

struct Ast_Note : Ast {
	bool is_global = false;
	const char* name = NULL;
	vector<Ast_Expression*> arguments;
};

enum Ast_Statement_Type {
	AST_STATEMENT_UNDEFINED = 0,
	AST_STATEMENT_BLOCK,
	AST_STATEMENT_IF,
	AST_STATEMENT_WHILE,
	AST_STATEMENT_BREAK,
	AST_STATEMENT_DECLARATION,
	AST_STATEMENT_RETURN,
	AST_STATEMENT_IMPORT,
	AST_STATEMENT_EXPRESSION,
};

struct Ast_Statement : Ast {
	Ast_Statement_Type stm_type = AST_STATEMENT_UNDEFINED;

	vector<Ast_Note*> notes;

	Ast_Note* get_note (const char* name);
	Ast_Note* remove_note (const char* name);
};

struct Ast_Import : Ast_Statement {
	Ast_Expression* target;

	Ast_Import (Ast_Expression* target = NULL) {
		this->stm_type = AST_STATEMENT_IMPORT;
		this->target = target;
	}
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

	bool is_global () { return !this->parent && !this->scope_of; }

	Ast_Declaration* find_declaration (const char* name, bool recurse = true, bool is_out_scope = false);
	Ast_Declaration* find_non_const_declaration (const char* name);
	Ast_Declaration* find_const_declaration (const char* name);
	bool is_ancestor (Ast_Block* other);
	Ast_Function* get_function ();
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

struct Ast_Break : Ast_Statement {
	Ast_Break () { this->stm_type = AST_STATEMENT_BREAK; }
};

const uint8_t AST_DECL_FLAG_CONSTANT = 0x1;

struct Ast_Declaration : Ast_Statement {
	const char* name = NULL;
	Ast_Expression* type = NULL;
	Ast_Expression* expression = NULL;

	uint8_t decl_flags = 0;
	Ast_Block* scope = NULL;

	// for struct property
	Ast_Struct_Type* _struct = NULL;
	size_t attribute_index = 0;
	size_t attribute_byte_offset = 0;

	// for bytecode
	int64_t bytecode_data_offset = -1;

	Ast_Declaration() { this->stm_type = AST_STATEMENT_DECLARATION; }

	bool is_constant () { return this->decl_flags & AST_DECL_FLAG_CONSTANT; }
	bool is_global () { return this->scope->is_global(); }
};

struct Ast_Return : Ast_Statement {
	Ast_Expression* exp = NULL;
	Ast_Block* block = NULL;

	Ast_Return() { this->stm_type = AST_STATEMENT_RETURN; }
};

enum Ast_Expression_Type {
	AST_EXPRESSION_UNDEFINED = 0,
	AST_EXPRESSION_TYPE_INSTANCE,
	AST_EXPRESSION_FUNCTION,
	AST_EXPRESSION_BINARY,
	AST_EXPRESSION_UNARY,
	AST_EXPRESSION_CALL,
	AST_EXPRESSION_IDENT,
	AST_EXPRESSION_LITERAL,
	AST_EXPRESSION_CAST,
};

struct Ast_Expression : Ast_Statement {
	Ast_Expression_Type exp_type = AST_EXPRESSION_UNDEFINED;
	Ast_Type_Instance* inferred_type = NULL;

	// for bytecode
	int8_t reg = -1;

	Ast_Expression() { this->stm_type = AST_STATEMENT_EXPRESSION; }
};

struct Ast_Cast : Ast_Expression {
	Ast_Expression* value = NULL;
	Ast_Expression* cast_to = NULL;

	Ast_Cast() { this->exp_type = AST_EXPRESSION_CAST; }
};

enum Ast_Type_Instance_Type {
	AST_TYPEDEF_UNDEFINED = 0,
	AST_TYPEDEF_FUNCTION,
	AST_TYPEDEF_STRUCT,
	AST_TYPEDEF_POINTER,
	AST_TYPEDEF_ARRAY,
};

#define AST_POINTER_SIZE sizeof(void*)

struct Ast_Type_Instance : Ast_Expression {
	Ast_Type_Instance_Type typedef_type = AST_TYPEDEF_UNDEFINED;
	bool is_signed = false;
	size_t byte_size = 0;
	char* name = NULL;

	Ast_Type_Instance() { this->exp_type = AST_EXPRESSION_TYPE_INSTANCE; }
};

struct Ast_Struct_Type : Ast_Type_Instance {
	vector<Ast_Declaration*> attributes;
	bool is_slice = false;

	Ast_Struct_Type(char* name = NULL, size_t byte_size = 0) {
		this->typedef_type = AST_TYPEDEF_STRUCT;
		this->byte_size = byte_size;
		this->name = name;
	}

	Ast_Declaration* find_attribute (const char* name);
};

struct Ast_Pointer_Type : Ast_Type_Instance {
	Ast_Expression* base = NULL;

	Ast_Pointer_Type(Ast_Expression* base = NULL) {
		this->typedef_type = AST_TYPEDEF_POINTER;
		this->byte_size = AST_POINTER_SIZE;
		this->base = base;
	}
};

struct Ast_Array_Type : Ast_Type_Instance {
	Ast_Expression* base = NULL;
	Ast_Expression* length_exp = NULL;

	uint64_t _length = 0;

	Ast_Array_Type() { this->typedef_type = AST_TYPEDEF_ARRAY; }

	uint64_t length ();
};

struct Ast_Slice_Type : Ast_Struct_Type {
	Ast_Expression* base = NULL;

	Ast_Slice_Type(Ast_Expression* base_type);
};

struct Ast_Function_Type : Ast_Type_Instance {
	vector<Ast_Expression*> arg_types;
	Ast_Expression* ret_type = NULL;

	Ast_Function_Type() {
		this->typedef_type = AST_TYPEDEF_FUNCTION;
		this->byte_size = AST_POINTER_SIZE;
	}
};

struct Ast_Function : Ast_Expression {
	const char* name = NULL;
	vector<Ast_Declaration*> arg_decls;
	Ast_Expression* ret_type = NULL;
	Ast_Block* scope = NULL;

	const char* foreign_module_name = NULL;
	const char* foreign_function_name = NULL;
	void* foreign_function_pointer = NULL;

	vector<Instruction*> bytecode;

	Ast_Function() { this->exp_type = AST_EXPRESSION_FUNCTION; }

	bool is_native () { return this->foreign_function_pointer; }
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
	AST_BINARY_REM,

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

Ast_Binary_Type token_to_binop (Token_Type type);

struct Ast_Binary : Ast_Expression {
	Ast_Binary_Type binary_op = AST_BINARY_UNINITIALIZED;
	Ast_Expression* lhs = NULL;
	Ast_Expression* rhs = NULL;

	Ast_Binary (Ast_Binary_Type type) {
		this->exp_type = AST_EXPRESSION_BINARY;
		this->binary_op = type;
	}

	Ast_Binary (Token_Type token_type) {
		this->exp_type = AST_EXPRESSION_BINARY;
		this->binary_op = token_to_binop(token_type);
	}

	static short get_precedence (Token_Type opToken);
	static bool is_left_associative (Token_Type opToken);
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

	Ast_Unary (Ast_Unary_Type type, Ast_Expression* exp = NULL) {
		this->exp_type = AST_EXPRESSION_UNARY;
		this->unary_op = type;
		this->exp = exp;
	}
};

struct Ast_Function_Call : Ast_Expression {
	Ast_Expression* fn;
	vector<Ast_Expression*> arguments;

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

	Ast_Block* scope = NULL;
	Ast_Declaration* declaration = NULL;

	// for bytecode
	bool is_in_register = false;

	Ast_Ident (Ast_Block* scope = NULL) {
		this->exp_type = AST_EXPRESSION_IDENT;
		this->scope = scope;
	}
};

void ast_compute_type_name_if_needed (Ast_Type_Instance* type_inst);
bool ast_type_are_equal (Ast_Type_Instance* type_inst1, Ast_Type_Instance* type_inst2);
Ast_Literal* ast_make_literal (const char* value);
Ast_Literal* ast_make_literal (unsigned long long value);
Ast_Ident* ast_make_ident (const char* name);
Ast_Unary* ast_make_unary (Ast_Unary_Type type, Ast_Expression* expression);
Ast_Binary* ast_make_binary (Ast_Binary_Type type, Ast_Expression* lhs, Ast_Expression* rhs);
Ast_Declaration* ast_make_declaration (const char* name, Ast_Expression* exp, bool is_const = true);
