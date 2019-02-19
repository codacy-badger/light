#pragma once

#include "platform.hpp"
#include "ast/types.hpp"
#include "utils/location.hpp"
#include "front/parser/lexer/token.hpp"
#include "utils/string_map.hpp"
#include "utils/string_vector.hpp"
#include "utils/array.hpp"

#include <assert.h>
#include <vector>
#include <algorithm>

#define INVALID_ARG_INDEX 500

struct Ast_Function;
struct Ast_Expression;
struct Ast_Declaration;
struct Ast_Scope;
struct Ast_Type;

struct Instruction;

struct Ast {
	Location location;
};

struct Ast_Arguments : Ast {
	std::vector<Ast_Expression*> unnamed;
	String_Map<Ast_Expression*> named;

    Ast_Expression* get_named_value (const char* param_name) {
	    for (auto entry : this->named) {
	        if (strcmp(entry.first, param_name) == 0) {
	            return entry.second;
	        }
	    }
	    return NULL;
	}

    Ast_Expression* get_unnamed_value (const size_t index) {
	    if (index < this->unnamed.size()) {
	        return this->unnamed[index];
	    } else return NULL;
	}
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

struct Ast_Statement : Ast {
	Ast_Statement_Type stm_type = AST_STATEMENT_UNDEFINED;
	uint16_t stm_flags = 0;
	Ast_Scope* parent_scope = NULL;
};

const uint16_t SCOPE_FLAG_FULLY_PARSED 		= 0x0001;
const uint16_t SCOPE_FLAG_IMPORTS_RESOLVED	= 0x0002;

struct Ast_Scope : Ast_Statement {
	uint16_t scope_flags = 0;

	std::vector<Ast_Statement*> statements;
	String_Map<Ast_Scope*> named_imports;
	std::vector<Ast_Scope*> imports;

	Ast_Scope* parent = NULL;
	Ast_Function* scope_of = NULL;

	Ast_Scope (Ast_Scope* parent = NULL) {
		this->stm_type = AST_STATEMENT_SCOPE;
		this->parent = parent;
	}

    bool is_global () { return this->parent == NULL; }

	std::vector<Ast_Statement*>::iterator find (Ast_Statement* stm) {
		return std::find(this->statements.begin(), this->statements.end(), stm);
	}

	std::vector<Ast_Statement*>::iterator add (std::vector<Ast_Statement*>::iterator it, Ast_Statement* stm) {
		stm->parent_scope = this;
		return this->statements.insert(it, stm);
	}

	std::vector<Ast_Statement*>::iterator add (std::vector<Ast_Statement*>::iterator it, Ast_Scope* scope) {
		return this->add(it, scope->statements);
	}

	std::vector<Ast_Statement*>::iterator add (std::vector<Ast_Statement*>::iterator it, std::vector<Ast_Statement*> others) {
		for (auto stm : others) {
			it = this->add(it, stm);
		}
		return it;
	}

	std::vector<Ast_Statement*>::iterator add (Ast_Statement* stm) {
		return this->add(this->statements.end(), stm);
	}

	std::vector<Ast_Statement*>::iterator add (std::vector<Ast_Statement*> others) {
		return this->add(this->statements.end(), others);
	}

	std::vector<Ast_Statement*>::iterator remove (Ast_Statement* stm) {
		return this->statements.erase(this->find(stm));
	}

	Ast_Scope* get_global_scope () {
		auto global_scope = this;
        while (global_scope->parent != NULL) {
            global_scope = global_scope->parent;
        }
		return global_scope;
	}

	bool has_unnamed_import (Ast_Scope* other) {
		for (auto imported_scope : this->imports) {
			if (imported_scope == other) return true;
		}
		return false;
	}

	bool all_dependencies_flagged (uint16_t flag) {
		for (auto imported_scope : this->imports) {
			if ((imported_scope->scope_flags & flag) != flag) return false;
		}
		for (auto entry : this->named_imports) {
			if ((entry.second->scope_flags & flag) != flag) return false;
		}
		return true;
	}

	bool are_all_imports_resolved () {
		return this->all_dependencies_flagged(SCOPE_FLAG_FULLY_PARSED | SCOPE_FLAG_IMPORTS_RESOLVED);
	}

	// @TODO this methods could be simplified, since most of the calls use
	// constants for the boolean flags. We have to be sure they works properly
	Ast_Declaration* find_declaration (const char* _name, bool use_imports, bool recurse);
	Ast_Declaration* find_const_declaration (const char* _name);
	Ast_Declaration* find_var_declaration (const char* _name);

	void find_all_declarations (String_Map<std::vector<Ast_Declaration*>>* decl_map);

	bool is_ancestor (Ast_Scope* other);
	Ast_Function* get_parent_function ();
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
	Array<Ast_Expression*> types;
	Array<Ast_Expression*> values;

    bool is_constant = false;
	Ast_Scope* scope = NULL;

	// for struct property
	size_t attribute_byte_offset = 0;

	// for bytecode
	int64_t bytecode_global_offset = -1;
	int64_t bytecode_stack_offset = -1;
	bool is_spilled = true;

	bool is_global () { return this->scope->is_global(); }

	Ast_Declaration() { this->stm_type = AST_STATEMENT_DECLARATION; }

	Ast_Declaration(const char* name, Ast_Expression* type, Ast_Expression* value = NULL) {
		this->stm_type = AST_STATEMENT_DECLARATION;
		this->values.push(value);
		this->names.push(name);
		this->types.push(type);
	}
};

struct Ast_Return : Ast_Statement {
	Ast_Expression* expression = NULL;
	Ast_Scope* scope = NULL;

	Ast_Return(Ast_Expression* expression = NULL) {
        this->stm_type = AST_STATEMENT_RETURN;
        this->expression = expression;
    }
};

struct Ast_Foreign : Ast_Statement {
    const char* module_name = NULL;
    const char* function_name = NULL;
	std::vector<Ast_Declaration*> declarations;

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
	int8_t reg = -1;

	Ast_Expression() { this->stm_type = AST_STATEMENT_EXPRESSION; }
};

struct Ast_Comma_Separated : Ast_Expression {
	Array<Ast_Expression*> expressions;

	Ast_Comma_Separated() { this->exp_type = AST_EXPRESSION_COMMA_SEPARATED; }
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
	std::vector<Instruction*> bytecode;

	Ast_Run (Ast_Expression* expression = NULL) {
		this->exp_type = AST_EXPRESSION_RUN;
		this->expression = expression;
	}
};

struct Ast_Cast : Ast_Expression {
	Ast_Expression* value;
	Ast_Expression* cast_to;

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
};

struct Ast_Type : Ast_Expression {
	Ast_Type_Type typedef_type = AST_TYPEDEF_UNDEFINED;
	const char* name = NULL;

	bool is_primitive = false;
	bool is_number = false;
	bool is_signed = false;

	size_t byte_size = 0;
	size_t byte_padding = 0;

	uint64_t guid = 0;

	Ast_Type() { this->exp_type = AST_EXPRESSION_TYPE; }

	bool can_be_in_register (uint8_t register_size) { return this->is_primitive && (this->byte_size <= register_size); }
};

const uint16_t STRUCT_FLAG_BEING_CHECKED 	= 0x0001;
const uint16_t STRUCT_FLAG_IDENTS_RESOLVED 	= 0x0002;

struct Ast_Struct_Type : Ast_Type {
	uint16_t struct_flags = 0;

	Ast_Scope scope;
	bool is_slice = false;

	Ast_Struct_Type(char* name = NULL, size_t byte_size = 0, bool is_primitive = false,
			bool is_number = false, bool is_signed = false) {
		this->typedef_type = AST_TYPEDEF_STRUCT;
		this->is_primitive = is_primitive;
		this->is_number = is_number;
		this->is_signed = is_signed;
		this->byte_size = byte_size;
		this->name = name;
	}

	Ast_Declaration* find_attribute (const char* attribute_name) {
		return this->scope.find_declaration(attribute_name, true, false);
	}
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

	Ast_Type* get_base_type_recursive();
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

	Ast_Slice_Type(Ast_Expression* base_type, const char* name = NULL);

	Ast_Expression* get_base() {
		auto attr_decl = this->find_attribute("data");
		auto ptr_type = static_cast<Ast_Pointer_Type*>(attr_decl->types[0]);
		return ptr_type->base;
	}

	Ast_Expression** get_base_ptr() {
		auto attr_decl = this->find_attribute("data");
		auto ptr_type = static_cast<Ast_Pointer_Type*>(attr_decl->types[0]);
		return &ptr_type->base;
	}

	Ast_Type* get_typed_base() {
		return static_cast<Ast_Type*>(this->get_base());
	}

	Ast_Type** get_typed_base_ptr() {
		return reinterpret_cast<Ast_Type**>(this->get_base_ptr());
	}
};

struct Ast_Function_Type : Ast_Type {
	std::vector<Ast_Expression*> arg_types;
	std::vector<Ast_Expression*> ret_types;

	Ast_Function_Type() {
		this->typedef_type = AST_TYPEDEF_FUNCTION;
		this->is_primitive = true;
	}
};

const uint16_t FUNCTION_FLAG_BEING_CHECKED 	= 0x0001;

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
	std::vector<Instruction*> bytecode;

	Ast_Function() { this->exp_type = AST_EXPRESSION_FUNCTION; }

	bool is_native () { return !!this->foreign_function_name; }

	Ast_Function_Type* build_function_type () {
		auto func_type = new Ast_Function_Type();

		for (auto stm : this->arg_scope->statements) {
			assert(stm->stm_type == AST_STATEMENT_DECLARATION);
			auto decl = static_cast<Ast_Declaration*>(stm);
			func_type->arg_types.push_back(decl->types[0]);
		}

		if (this->ret_scope->statements.size() > 0) {
			for (auto stm : this->ret_scope->statements) {
				assert(stm->stm_type == AST_STATEMENT_DECLARATION);
				auto decl = static_cast<Ast_Declaration*>(stm);
				func_type->ret_types.push_back(decl->types[0]);
			}
		} else func_type->ret_types.push_back(Types::type_void);

		return func_type;
	}

	size_t get_arg_index (const char* _name) {
		for (size_t i = 0; i < this->arg_scope->statements.size(); i++) {
			auto stm = this->arg_scope->statements[i];

			assert(stm->stm_type == AST_STATEMENT_DECLARATION);
			auto decl = static_cast<Ast_Declaration*>(stm);

			assert(decl->names.size > 0);
			if (strcmp(decl->names[0], _name) == 0) return i;
		}
		return INVALID_ARG_INDEX;
	}

	Ast_Declaration* get_arg_declaration (size_t index) {
		assert(this->arg_scope->statements.size() > index);

		auto arg_stm = this->arg_scope->statements[index];
		assert(arg_stm->stm_type == AST_STATEMENT_DECLARATION);
		return static_cast<Ast_Declaration*>(arg_stm);
	}

	Ast_Expression* get_default_value (size_t index) {
		auto arg_decl = this->get_arg_declaration(index);
		assert(arg_decl->values.size > 0);
		return arg_decl->values[0];
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

Ast_Binary_Type token_to_binop (Token_Type type);

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
		this->binary_op = token_to_binop(token_type);
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
	Ast_Expression* func;
	Ast_Arguments* arguments;

	Ast_Function_Call() { this->exp_type = AST_EXPRESSION_CALL; }
};

struct Ast_Ident : Ast_Expression {
	const char* name = NULL;

    // for symbol resolution
	Ast_Declaration* declaration = NULL;
	Ast_Scope* scope = NULL;

	Ast_Ident () { this->exp_type = AST_EXPRESSION_IDENT; }
};
