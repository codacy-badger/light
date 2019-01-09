#pragma once

#include "module.hpp"
#include "platform.hpp"
#include "report.hpp"
#include "phase/imp/lexer/token.hpp"

#include <vector>
#include <map>

struct Ast_Ident;
struct Ast_Function;
struct Ast_Expression;
struct Ast_Declaration;
struct Ast_Struct_Type;
struct Ast_Type_Instance;

struct Instruction;

using namespace std;

struct cmp_str {
   bool operator()(char const *a, char const *b) const {
      return std::strcmp(a, b) < 0;
   }
};

#define WARN_MAX_DEREF_COUNT 3

struct Ast {
	Location location;

    static uint64_t node_count;

    Ast () { node_count++; }

    ~Ast () { node_count--; }
};

struct Ast_Arguments : Ast {
	vector<Ast_Expression*> unnamed;
	map<const char*, Ast_Expression*, cmp_str> named;

	bool add (Ast_Expression* exp);
    Ast_Expression* get_named_value (const char* param_name);
    Ast_Expression* get_unnamed_value (const size_t index);
};

enum Ast_Statement_Type {
	AST_STATEMENT_UNDEFINED = 0,
	AST_STATEMENT_SCOPE,
	AST_STATEMENT_IF,
	AST_STATEMENT_WHILE,
	AST_STATEMENT_BREAK,
	AST_STATEMENT_DECLARATION,
	AST_STATEMENT_RETURN,
	AST_STATEMENT_EXPRESSION,
};

struct Ast_Statement : Ast {
	Ast_Statement_Type stm_type = AST_STATEMENT_UNDEFINED;

	vector<const char*> notes;
    bool remove_from_scope = false;

	bool remove_note (const char* name);
};

struct Ast_Scope : Ast_Statement {
	vector<Ast_Statement*> statements;
	vector<Ast_Scope*> import_scopes;

	Ast_Scope* parent = NULL;
	Ast_Function* scope_of = NULL;

	// for symbol resolution
	vector<Ast_Ident*> unresolved_idents;

	Ast_Scope (Ast_Scope* parent = NULL) {
		this->stm_type = AST_STATEMENT_SCOPE;
		this->parent = parent;
	}

    bool is_global () { return this->parent == NULL; }

	void add (Ast_Statement* stm) {
		this->statements.push_back(stm);
	}

    Ast_Scope* get_global_scope () {
        auto global_scope = this;
        while (global_scope->parent != NULL) {
            global_scope = global_scope->parent;
        }
        return global_scope;
    }

    Ast_Declaration* find_global_declaration (const char* _name, bool is_const = false) {
        return this->get_global_scope()->find_declaration(_name, is_const);
    }

    Ast_Declaration* find_declaration (const char* _name, bool is_const = false);
    Ast_Declaration* find_external_declaration (const char* _name, bool is_const = false);

	Ast_Declaration* find_non_const_declaration (const char* name);
	Ast_Declaration* find_const_declaration (const char* name);
	Ast_Declaration* find_declaration (const char* name);
	bool is_ancestor (Ast_Scope* other);
	Ast_Function* get_parent_function ();
};

struct Ast_If : Ast_Statement {
	Ast_Expression* condition = NULL;
	Ast_Scope* then_scope = NULL;
	Ast_Scope* else_scope = NULL;

	Ast_If () { this->stm_type = AST_STATEMENT_IF; }
};

struct Ast_While : Ast_Statement {
	Ast_Expression* condition = NULL;
	Ast_Scope* scope = NULL;

	Ast_While () { this->stm_type = AST_STATEMENT_WHILE; }
};

struct Ast_Break : Ast_Statement {
	Ast_Break () { this->stm_type = AST_STATEMENT_BREAK; }
};

struct Ast_Declaration : Ast_Statement {
	const char* name = NULL;
	Ast_Expression* type = NULL;
	Ast_Expression* expression = NULL;

    bool is_constant = false;

	Ast_Scope* scope = NULL;

	// for struct property
	size_t attribute_byte_offset = 0;

	// for bytecode
	int64_t bytecode_data_offset = -1;
	bool is_spilled = false;

	Ast_Declaration() { this->stm_type = AST_STATEMENT_DECLARATION; }

	bool is_global () { return this->scope->is_global(); }
};

struct Ast_Return : Ast_Statement {
	Ast_Expression* expression = NULL;

	Ast_Return(Ast_Expression* expression = NULL) {
        this->stm_type = AST_STATEMENT_RETURN;
        this->expression = expression;
    }
};

enum Ast_Expression_Type {
	AST_EXPRESSION_UNDEFINED = 0,
	AST_EXPRESSION_TYPE_INSTANCE,
	AST_EXPRESSION_DIRECTIVE,
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

enum Ast_Directive_Type {
	AST_DIRECTIVE_UNDEFINED = 0,
	AST_DIRECTIVE_INCLUDE,
	AST_DIRECTIVE_IMPORT,
	AST_DIRECTIVE_FOREIGN,
	AST_DIRECTIVE_RUN,
	AST_DIRECTIVE_IF,
};

struct Ast_Directive : Ast_Expression {
	Ast_Directive_Type dir_type = AST_DIRECTIVE_UNDEFINED;

	Ast_Directive () { this->exp_type = AST_EXPRESSION_DIRECTIVE; }
};

struct Ast_Directive_Import : Ast_Directive {
	const char* path = NULL;

	char absolute_path[MAX_PATH_LENGTH];

	Ast_Directive_Import () { this->dir_type = AST_DIRECTIVE_IMPORT; }
};

struct Ast_Directive_Foreign : Ast_Directive {
    const char* module_name = NULL;
    const char* function_name = NULL;
	vector<Ast_Declaration*> declarations;

	Ast_Directive_Foreign () { this->dir_type = AST_DIRECTIVE_FOREIGN; }

    void add (Ast_Statement* stm);
    void add (Ast_Declaration* decl);

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

struct Ast_Directive_Run : Ast_Directive {
	Ast_Expression* expression = NULL;

	// for bytecode
	vector<Instruction*> bytecode;

	Ast_Directive_Run () { this->dir_type = AST_DIRECTIVE_RUN; }
};

struct Ast_Directive_If : Ast_Directive {
	Ast_If* stm_if = NULL;

	Ast_Directive_If () { this->dir_type = AST_DIRECTIVE_IF; }
};

struct Ast_Cast : Ast_Expression {
	Ast_Expression* value = NULL;
	Ast_Expression* cast_to = NULL;

	bool is_array_to_slice_cast = false;
    bool is_value_to_any_cast = false;

	Ast_Cast() { this->exp_type = AST_EXPRESSION_CAST; }
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
		const char* string_value;
	};

	// for bytecode
	size_t data_offset = 0;

	Ast_Literal () { this->exp_type = AST_EXPRESSION_LITERAL; }

	bool is_hexadecimal () 	{ return string_value[0] == '0' && string_value[1] == 'x'; }
	bool is_binary () 		{ return string_value[0] == '0' && string_value[1] == 'b'; }
	bool is_decimal () 		{ return strstr(string_value, ".") != NULL; }
	bool as_boolean () 		{ return uint_value != 0; }
};

enum Ast_Type_Instance_Type {
	AST_TYPEDEF_UNDEFINED = 0,
	AST_TYPEDEF_FUNCTION,
	AST_TYPEDEF_STRUCT,
	AST_TYPEDEF_POINTER,
	AST_TYPEDEF_ARRAY,
};

struct Ast_Type_Instance : Ast_Expression {
	Ast_Type_Instance_Type typedef_type = AST_TYPEDEF_UNDEFINED;
	const char* name = NULL;

	bool is_signed = false;
	bool is_primitive = false;

	size_t byte_size = 0;
	size_t byte_padding = 0;

	int64_t guid = -1;

	Ast_Type_Instance() { this->exp_type = AST_EXPRESSION_TYPE_INSTANCE; }

	bool can_be_in_register (uint8_t register_size) { return this->is_primitive && (this->byte_size <= register_size); }
};

struct Ast_Struct_Type : Ast_Type_Instance {
	vector<Ast_Declaration*> attributes;
	bool is_slice = false;

	Ast_Struct_Type(char* name = NULL, size_t byte_size = 0, bool is_primitive = false, bool is_signed = false) {
		this->typedef_type = AST_TYPEDEF_STRUCT;
		this->is_primitive = is_primitive;
		this->is_signed = is_signed;
		this->byte_size = byte_size;
		this->name = name;
	}

	Ast_Declaration* find_attribute (const char* name);
};

struct Ast_Pointer_Type : Ast_Type_Instance {
	Ast_Expression* base = NULL;

	Ast_Pointer_Type(Ast_Expression* base = NULL) {
		this->typedef_type = AST_TYPEDEF_POINTER;
		this->is_primitive = true;
		this->base = base;
	}

	Ast_Type_Instance* get_base_type_recursive();
};

struct Ast_Array_Type : Ast_Type_Instance {
	Ast_Expression* base = NULL;
	Ast_Expression* length = NULL;

	uint64_t length_as_number = 0;

	Ast_Array_Type() { this->typedef_type = AST_TYPEDEF_ARRAY; }

	uint64_t get_length () {
    	if (!this->length_as_number) {
            if (this->length->exp_type == AST_EXPRESSION_LITERAL) {
                auto literal = static_cast<Ast_Literal*>(this->length);
                this->length_as_number = literal->uint_value;
            }
    	}
    	return this->length_as_number;
    }
};

struct Ast_Slice_Type : Ast_Struct_Type {

	Ast_Slice_Type(Ast_Expression* base_type, const char* name = NULL);

	Ast_Expression* get_base() {
		auto attr_decl = this->find_attribute("data");
		auto ptr_type = static_cast<Ast_Pointer_Type*>(attr_decl->type);
		return ptr_type->base;
	}

	Ast_Type_Instance* get_typed_base() {
		return static_cast<Ast_Type_Instance*>(this->get_base());
	}
};

struct Ast_Function_Type : Ast_Type_Instance {
	vector<Ast_Declaration*> arg_decls;
	Ast_Expression* ret_type = NULL;

	Ast_Function_Type() { this->typedef_type = AST_TYPEDEF_FUNCTION; }

    Ast_Declaration* get_declaration (const char* decl_name) {
        for (auto decl : this->arg_decls) {
            if (strcmp(decl->name, decl_name) == 0) {
                return decl;
            }
        }
        return NULL;
    }

	size_t count_arguments_without_defaults () {
		size_t count = 0;
		for (auto decl : this->arg_decls) {
			if (!decl->expression) {
				count++;
			}
		}
		return count;
	}
};

struct Ast_Function : Ast_Expression {
	const char* name = NULL;
	Ast_Function_Type* type = NULL;
	Ast_Scope* scope = NULL;

	// for foreign functions
	const char* foreign_module_name = NULL;
	const char* foreign_function_name = NULL;
	void* foreign_function_pointer = NULL;

	// for bytecode
	vector<Instruction*> bytecode;

	Ast_Function() { this->exp_type = AST_EXPRESSION_FUNCTION; }

	bool is_native () { return this->foreign_function_pointer; }
};

enum Ast_Binary_Type {
	AST_BINARY_UNINITIALIZED = 0,
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

	Ast_Type_Instance* get_result_type();

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
	Ast_Expression* func;
	Ast_Arguments* arguments;

	Ast_Function_Call() { this->exp_type = AST_EXPRESSION_CALL; }
};

struct Ast_Ident : Ast_Expression {
	const char* name = NULL;

    // for symbol resolution
	Ast_Declaration* declaration = NULL;

	Ast_Ident () { this->exp_type = AST_EXPRESSION_IDENT; }
};

Ast_Struct_Type* build_type_any();

void ast_compute_type_name_if_needed (Ast_Type_Instance* type_inst);
bool ast_function_types_are_equal (Ast_Function_Type* func_type1, Ast_Function_Type* func_type2);
bool ast_types_are_equal (Ast_Type_Instance* type_inst1, Ast_Type_Instance* type_inst2);

bool try_cast (Ast_Expression** exp_ptr, Ast_Type_Instance* type_from, Ast_Type_Instance* type_to);
bool try_cast (Ast_Expression** exp_ptr, Ast_Type_Instance* type_to);

Ast_Type_Instance* ast_get_container_signed (Ast_Type_Instance* unsigned_type);
Ast_Struct_Type* ast_get_smallest_type (uint64_t value);
Ast_Struct_Type* ast_get_smallest_type (int64_t value);

Ast_Literal* ast_make_literal (const char* value);
Ast_Literal* ast_make_literal (unsigned long long value);
Ast_Literal* ast_make_literal (bool value);
Ast_Ident* ast_make_ident (const char* name);
Ast_Unary* ast_make_unary (Ast_Unary_Type type, Ast_Expression* expression);
Ast_Binary* ast_make_binary (Ast_Binary_Type type, Ast_Expression* lhs, Ast_Expression* rhs);
Ast_Declaration* ast_make_declaration (const char* name, Ast_Expression* exp, bool is_const = true);
Ast_Declaration* ast_make_declaration_with_type (const char* name, Ast_Expression* type);
