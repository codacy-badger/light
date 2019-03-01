#pragma once

#include "platform.hpp"
#include "utils/location.hpp"
#include "parser/lexer/token.hpp"
#include "utils/string_map.hpp"
#include "utils/array.hpp"

#include <assert.h>

struct Ast_Comma_Separated;
struct Ast_Declaration;
struct Ast_Expression;
struct Ast_Function;
struct Ast_Scope;
struct Ast_Type;

struct Instruction;

struct Ast {
	Location location;
	size_t ast_guid = 0;
};

enum Ast_Statement_Type {
	AST_STATEMENT_UNDEFINED = 0,
	AST_STATEMENT_SCOPE,
	AST_STATEMENT_DEFER,
	AST_STATEMENT_ASSIGN,
	AST_STATEMENT_IF,
	AST_STATEMENT_WHILE,
	AST_STATEMENT_BREAK,
	AST_STATEMENT_DECLARATION,
	AST_STATEMENT_RETURN,
	AST_STATEMENT_STATIC_IF,
	AST_STATEMENT_FOREIGN,
	AST_STATEMENT_EXPRESSION,
};

// @Info this flag only applies to global statements, 
// since those are the only ones that we have to handle
// in special order
const uint16_t STM_FLAG_IDENTS_RESOLVED 	= 0x0001;
const uint16_t STM_FLAG_STATIC_IFS_RESOLVED = 0x0002;

struct Ast_Statement : Ast {
	Ast_Statement_Type stm_type = AST_STATEMENT_UNDEFINED;
	Ast_Scope* parent_scope = NULL;
	uint16_t stm_flags = 0;
};

const uint16_t SCOPE_FLAG_FULLY_PARSED 		= 0x0001;
const uint16_t SCOPE_FLAG_IMPORTS_RESOLVED	= 0x0002;
const uint16_t SCOPE_FLAG_TYPES_CHECKED		= 0x0004;

struct Ast_Scope : Ast_Statement {
	uint16_t scope_flags = 0;

	Array<Ast_Statement*> statements;
	Array<Ast_Scope*> imports;

	Ast_Scope* parent = NULL;
	Ast_Function* scope_of = NULL;

	size_t stack_byte_padding = 0;
	size_t stack_byte_size = 0;

	Ast_Scope (Ast_Scope* parent = NULL) {
		this->stm_type = AST_STATEMENT_SCOPE;
		this->parent = parent;
	}

    bool is_global () { return this->parent == NULL; }

	inline
	void on_stm_add (Ast_Statement* stm) {
		stm->parent_scope = this;
	}

	void add (size_t index, Ast_Statement* stm) {
		this->statements.insert(index, stm);
		this->on_stm_add(stm);
	}

	void add (size_t index, Array<Ast_Statement*>* others) {
		For (*others) {
			this->add(index, it);
		}
	}

	void add (Ast_Statement* stm) {
		this->statements.push(stm);
		this->on_stm_add(stm);
	}

	size_t find (Ast_Statement* stm) {
		return this->statements.find(stm);
	}

	size_t remove (Ast_Statement* stm) {
		return this->statements.remove(stm);
	}
};

struct Ast_Assign : Ast_Statement {
	Ast_Expression* variable = NULL;
	Ast_Expression* value = NULL;

	Ast_Assign () { this->stm_type = AST_STATEMENT_ASSIGN; }
};

struct Ast_Defer : Ast_Statement {
	Ast_Statement* statement;

	Ast_Defer (Ast_Statement* statement = NULL) {
		this->stm_type = AST_STATEMENT_DEFER;
		this->statement = statement;
	}
};

struct Ast_If : Ast_Statement {
	Ast_Expression* condition = NULL;
	Ast_Scope* then_body = NULL;
	Ast_Scope* else_body = NULL;

	Ast_If () { this->stm_type = AST_STATEMENT_IF; }
};

struct Ast_While : Ast_Statement {
	Ast_Expression* condition = NULL;
	Ast_Scope* body = NULL;

	Ast_While () { this->stm_type = AST_STATEMENT_WHILE; }
};

struct Ast_Break : Ast_Statement {
	Ast_Break () { this->stm_type = AST_STATEMENT_BREAK; }
};

struct Ast_Declaration : Ast_Statement {
	Array<const char*> names;
	union {
		Ast_Expression* type;
		Ast_Type* typed_type;
	};
	Ast_Expression* value;

    bool is_constant = false;

	// for struct property
	size_t attribute_byte_offset = 0;

	// for bytecode
	int64_t global_offset = -1;
	int64_t reg = -1;

	bool is_global () { return !this->parent_scope || this->parent_scope->is_global(); }

	Ast_Declaration() { this->stm_type = AST_STATEMENT_DECLARATION; }

	Ast_Declaration(const char* name, Ast_Expression* type, Ast_Expression* value = NULL) {
		this->stm_type = AST_STATEMENT_DECLARATION;
		this->names.push(name);
		this->value = value;
		this->type = type;
	}
};

struct Ast_Return : Ast_Statement {
	Ast_Comma_Separated* result = NULL;
	Ast_Scope* scope = NULL;

	Ast_Return(Ast_Comma_Separated* result = NULL) {
        this->stm_type = AST_STATEMENT_RETURN;
        this->result = result;
    }
};

struct Ast_Foreign : Ast_Statement {
    const char* module_name = NULL;
    const char* function_name = NULL;
	Array<Ast_Declaration*> declarations;

	Ast_Foreign () { this->stm_type = AST_STATEMENT_FOREIGN; }

    const char* get_foreign_module_name_from_file () {
        if (this->location.filename != NULL) {
            auto file_name = _strdup(os_get_file_part(this->location.filename));
            file_name[strlen(file_name) - 3] = '\0';
            return file_name;
        } else return NULL;
    }

    const char* get_foreign_function_name (const char* current_function_name) {
        if (!this->function_name) {
            return current_function_name;
        } else return this->function_name;
    }
};

struct Ast_Static_If : Ast_Statement {
	Ast_If* stm_if = NULL;

	Ast_Static_If (Ast_If* stm_if = NULL) {
		this->stm_type = AST_STATEMENT_STATIC_IF;
		this->stm_if = stm_if;
	}
};

enum Ast_Expression_Type {
	AST_EXPRESSION_UNDEFINED = 0,
	AST_EXPRESSION_COMMA_SEPARATED,
	AST_EXPRESSION_TYPE,
	AST_EXPRESSION_RUN,
	AST_EXPRESSION_FUNCTION,
	AST_EXPRESSION_BINARY,
	AST_EXPRESSION_UNARY,
	AST_EXPRESSION_CALL,
	AST_EXPRESSION_IDENT,
	AST_EXPRESSION_LITERAL,
	AST_EXPRESSION_CAST,
	AST_EXPRESSION_IMPORT,
};

struct Ast_Expression : Ast_Statement {
	Ast_Expression_Type exp_type = AST_EXPRESSION_UNDEFINED;
	Ast_Type* inferred_type = NULL;

	// for bytecode
	int64_t reg = -1;

	Ast_Expression() { this->stm_type = AST_STATEMENT_EXPRESSION; }
};

struct Ast_Comma_Separated : Ast_Expression {
	Array<Ast_Expression*> expressions;
	String_Map<Ast_Expression*> named_expressions;

	Ast_Comma_Separated() { this->exp_type = AST_EXPRESSION_COMMA_SEPARATED; }

    Ast_Expression* get_unnamed_value (const size_t index) {
	    if (index < this->expressions.size) {
	        return this->expressions[index];
	    } else return NULL;
	}
};

struct Ast_Import : Ast_Expression {
	char path[MAX_PATH_LENGTH];
	bool is_include = false;
	Ast_Scope* scope = NULL;

	char resolved_source_file[MAX_PATH_LENGTH];
	Ast_Scope* file_scope = NULL;

	Ast_Import() { this->exp_type = AST_EXPRESSION_IMPORT; }
};

struct Ast_Run : Ast_Expression {
	Ast_Expression* expression = NULL;

	// for bytecode
	Array<Instruction*> bytecode;

	Ast_Run (Ast_Expression* expression = NULL) {
		this->exp_type = AST_EXPRESSION_RUN;
		this->expression = expression;
	}
};

struct Ast_Cast : Ast_Expression {
	Ast_Expression* value;
	union {
		Ast_Expression* cast_to = NULL;
		Ast_Type* typed_cast_to;
	};

	bool is_array_to_slice_cast = false;
    bool is_value_to_any_cast = false;

	Ast_Cast (Ast_Expression* value = NULL, Ast_Expression* cast_to = NULL) {
		this->exp_type = AST_EXPRESSION_CAST;
		this->cast_to = cast_to;
		this->value = value;
	}
};

enum Ast_Literal_Type {
	AST_LITERAL_UNINITIALIZED,
	AST_LITERAL_SIGNED_INT,
	AST_LITERAL_UNSIGNED_INT,
	AST_LITERAL_DECIMAL,
	AST_LITERAL_STRING,
	AST_LITERAL_BOOL,
};

struct Ast_Literal : Ast_Expression {
	Ast_Literal_Type literal_type = AST_LITERAL_UNINITIALIZED;

	union {
		bool 		bool_value;
		int64_t  	int_value;
		uint64_t  	uint_value;
		double 		decimal_value;
		const char* string_value;
	};

	// for bytecode
	size_t data_offset = 0;

	Ast_Literal () { this->exp_type = AST_EXPRESSION_LITERAL; }

	Ast_Literal (bool value) {
		this->exp_type = AST_EXPRESSION_LITERAL;
		this->literal_type = AST_LITERAL_BOOL;
		this->bool_value = value;
	}

	Ast_Literal (uint64_t value) {
		this->exp_type = AST_EXPRESSION_LITERAL;
		this->literal_type = AST_LITERAL_UNSIGNED_INT;
		this->uint_value = value;
	}

	Ast_Literal (int64_t value) {
		this->exp_type = AST_EXPRESSION_LITERAL;
		this->literal_type = AST_LITERAL_SIGNED_INT;
		this->int_value = value;
	}

	Ast_Literal (const char* value) {
		this->exp_type = AST_EXPRESSION_LITERAL;
		this->literal_type = AST_LITERAL_STRING;
		this->string_value = value;
	}

	bool is_hexadecimal () 	{ return string_value[0] == '0' && string_value[1] == 'x'; }
	bool is_binary () 		{ return string_value[0] == '0' && string_value[1] == 'b'; }
	bool is_decimal () 		{ return strstr(string_value, ".") != NULL; }
	bool as_boolean () 		{ return uint_value != 0; }
};

enum Ast_Type_Type {
	AST_TYPEDEF_UNDEFINED = 0,
	AST_TYPEDEF_FUNCTION,
	AST_TYPEDEF_STRUCT,
	AST_TYPEDEF_POINTER,
	AST_TYPEDEF_ARRAY,
	AST_TYPEDEF_SLICE,
	AST_TYPEDEF_TUPLE,
};

const uint16_t TYPE_FLAG_TYPE_CHECKED	= 0x0001;
const uint16_t TYPE_FLAG_SIZED 			= 0x0002;

struct Ast_Type : Ast_Expression {
	Ast_Type_Type typedef_type = AST_TYPEDEF_UNDEFINED;
	const char* name = NULL;
	
	uint16_t type_flags = 0;

	bool is_primitive = false;
	bool is_number = false;
	bool is_signed = false;

	size_t byte_size = 0;
	size_t byte_padding = 0;

	uint64_t type_guid = 0;

	Ast_Type() { this->exp_type = AST_EXPRESSION_TYPE; }

	bool can_be_in_register (uint8_t register_size) { return this->is_primitive && (this->byte_size <= register_size); }
};

const uint16_t STRUCT_FLAG_BEING_CHECKED 	= 0x0001;

struct Ast_Struct_Type : Ast_Type {
	uint16_t struct_flags = 0;

	Ast_Scope scope;
	bool is_slice = false;

	Ast_Struct_Type(const char* name = NULL, size_t byte_size = 0, bool is_primitive = false,
			bool is_number = false, bool is_signed = false) {
		this->typedef_type = AST_TYPEDEF_STRUCT;
		this->is_primitive = is_primitive;
		this->is_number = is_number;
		this->is_signed = is_signed;
		this->byte_size = byte_size;
		this->name = name;

		if (byte_size > 0) this->type_flags |= TYPE_FLAG_SIZED;
	}

	bool is_global () { return !this->scope.parent || this->scope.parent->is_global(); }
};

struct Ast_Pointer_Type : Ast_Type {
	union {
		Ast_Expression* base = NULL;
		Ast_Type* typed_base;
	};

	Ast_Pointer_Type(Ast_Expression* base = NULL) {
		this->typedef_type = AST_TYPEDEF_POINTER;
		this->is_primitive = true;
		this->base = base;
	}

	Ast_Type* get_base_type_recursive() {
		if (this->typed_base->typedef_type != AST_TYPEDEF_POINTER) {
			return this->typed_base;
		}

		auto tmp = this;
		while (tmp->typed_base->typedef_type == AST_TYPEDEF_POINTER) {
			tmp = static_cast<Ast_Pointer_Type*>(tmp->typed_base);
			if (tmp->typed_base->typedef_type != AST_TYPEDEF_POINTER) {
				return tmp->typed_base;
			}
		}

		return NULL;
	}
};

struct Ast_Array_Type : Ast_Type {
	union {
		Ast_Expression* base = NULL;
		Ast_Type* typed_base;
	};
	Ast_Expression* length;

	uint64_t length_uint = 0;

	Ast_Array_Type(Ast_Expression* base = NULL, Ast_Expression* length = NULL) {
		this->typedef_type = AST_TYPEDEF_ARRAY;
		this->length = length;
		this->base = base;
	}
};

struct Ast_Slice_Type : Ast_Struct_Type {
	union {
		Ast_Expression* base = NULL;
		Ast_Type* typed_base;
	};

	Ast_Slice_Type(Ast_Expression* base_type, const char* name = NULL) {
		this->typedef_type = AST_TYPEDEF_SLICE;
		this->base = base_type;
		this->is_slice = true;
		this->name = name;
	}
};

struct Ast_Tuple_Type : Ast_Type {
	union {
		Array<Ast_Expression*> types = Array<Ast_Expression*>();
		Array<Ast_Type*> typed_types;
	};

	Ast_Tuple_Type() { this->typedef_type = AST_TYPEDEF_TUPLE; }
};

struct Ast_Function_Type : Ast_Type {
	Array<Ast_Expression*> arg_types = Array<Ast_Expression*>();
	union {
		Ast_Expression* ret_type = NULL;
		Ast_Type* typed_ret_type;
	};

	Ast_Function_Type() {
		this->typedef_type = AST_TYPEDEF_FUNCTION;
		this->is_primitive = true;
	}
};

const uint16_t FUNCTION_FLAG_BEING_CHECKED 			= 0x0001;

struct Ast_Function : Ast_Expression {
	uint16_t func_flags = 0;

	const char* name = NULL;

	Ast_Scope* arg_scope = NULL;
	Ast_Scope* ret_scope = NULL;

	Ast_Scope* body = NULL;

	// for foreign functions
	const char* foreign_module_name = NULL;
	const char* foreign_function_name = NULL;
	void* foreign_function_pointer = NULL;

	// for bytecode
	Array<Instruction*> bytecode;

	Ast_Function (const char* _name = NULL) {
		this->exp_type = AST_EXPRESSION_FUNCTION;
		this->name = _name;
	}

	bool is_native () {
		return this->foreign_function_pointer
			|| this->foreign_function_name;
	}
};

enum Ast_Binary_Type {
	AST_BINARY_UNINITIALIZED = 0,
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

struct Ast_Binary : Ast_Expression {
	Ast_Binary_Type binary_op = AST_BINARY_UNINITIALIZED;
	Ast_Expression* lhs = NULL;
	Ast_Expression* rhs = NULL;

	bool type_should_match = false;

	Ast_Binary (Ast_Binary_Type type) {
		this->exp_type = AST_EXPRESSION_BINARY;
		this->binary_op = type;
	}

	Ast_Binary (Token_Type token_type) {
		this->exp_type = AST_EXPRESSION_BINARY;
		switch (token_type) {
			case TOKEN_DOT: 			this->binary_op = AST_BINARY_ATTRIBUTE; break;
			case TOKEN_SQ_BRAC_OPEN: 	this->binary_op = AST_BINARY_SUBSCRIPT; break;
			
			case TOKEN_DOUBLE_AMP:		this->binary_op = AST_BINARY_LOGICAL_AND; break;
			case TOKEN_DOUBLE_PIPE:		this->binary_op = AST_BINARY_LOGICAL_OR; break;
			
			case TOKEN_ADD: 			this->binary_op = AST_BINARY_ADD; break;
			case TOKEN_SUB: 			this->binary_op = AST_BINARY_SUB; break;
			case TOKEN_MUL: 			this->binary_op = AST_BINARY_MUL; break;
			case TOKEN_DIV: 			this->binary_op = AST_BINARY_DIV; break;
			case TOKEN_PERCENT:			this->binary_op = AST_BINARY_REM; break;
			
			case TOKEN_AMP:				this->binary_op = AST_BINARY_BITWISE_AND; break;
			case TOKEN_PIPE:			this->binary_op = AST_BINARY_BITWISE_OR; break;
			case TOKEN_CARET:			this->binary_op = AST_BINARY_BITWISE_XOR; break;
			case TOKEN_RIGHT_SHIFT:		this->binary_op = AST_BINARY_BITWISE_RIGHT_SHIFT; break;
			case TOKEN_LEFT_SHIFT:		this->binary_op = AST_BINARY_BITWISE_LEFT_SHIFT; break;
			
			case TOKEN_DOUBLE_EQUAL:	this->binary_op = AST_BINARY_EQ; break;
			case TOKEN_NOT_EQUAL:		this->binary_op = AST_BINARY_NEQ; break;
			case TOKEN_GREATER_EQUAL:	this->binary_op = AST_BINARY_GTE; break;
			case TOKEN_LESSER_EQUAL:	this->binary_op = AST_BINARY_LTE; break;
			case TOKEN_GREATER:			this->binary_op = AST_BINARY_GT; break;
			case TOKEN_LESSER:			this->binary_op = AST_BINARY_LT; break;

			default: 					assert(false);
		}
	}
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
	union {
		Ast_Expression* func = NULL;
		Ast_Function* typed_func;
	};
	Ast_Comma_Separated* arguments;

	Array<int64_t> ret_regs;

	Ast_Function_Call() { this->exp_type = AST_EXPRESSION_CALL; }
};

struct Ast_Ident : Ast_Expression {
	const char* name = NULL;

    // for symbol resolution
	Ast_Declaration* declaration = NULL;
	Ast_Scope* scope = NULL;

	Ast_Ident () { this->exp_type = AST_EXPRESSION_IDENT; }
};
