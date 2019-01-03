#pragma once

#include "ast/ast.hpp"
#include "platform.hpp"

#include <stdint.h>

struct Pipeline;

#define PRINT_METRIC(format, ...) printf("        " format "\n", __VA_ARGS__)

struct Pipe {
	const char* pipe_name = NULL;
	uint64_t start_time = 0;
	double total_time = 0;

	bool stop_processing = false;

	Pipeline* pipeline = NULL;
	size_t pipe_index = 0;

	virtual void process (Ast_Scope* scope) {
		this->start_time = os_get_user_time();

		this->handle(&scope);

		this->total_time += os_time_user_stop(this->start_time);
	}

	virtual void on_finish () { /* empty */ }

	virtual void print_pipe_metrics () { /* empty by default */ }

	virtual void handle (Ast_Statement** stm) {
		for (auto &note : (*stm)->notes) {
			this->handle(&note);
		}
		switch ((*stm)->stm_type) {
			case AST_STATEMENT_SCOPE: {
				this->handle(reinterpret_cast<Ast_Scope**>(stm));
				break;
			}
			case AST_STATEMENT_IF: {
				this->handle(reinterpret_cast<Ast_If**>(stm));
				break;
			}
			case AST_STATEMENT_WHILE: {
				this->handle(reinterpret_cast<Ast_While**>(stm));
				break;
			}
			case AST_STATEMENT_DECLARATION: {
				this->handle(reinterpret_cast<Ast_Declaration**>(stm));
				break;
			}
			case AST_STATEMENT_RETURN: {
				this->handle(reinterpret_cast<Ast_Return**>(stm));
				break;
			}
			case AST_STATEMENT_BREAK: {
				this->handle(reinterpret_cast<Ast_Break**>(stm));
				break;
			}
			case AST_STATEMENT_EXPRESSION: {
				this->handle(reinterpret_cast<Ast_Expression**>(stm));
				break;
			}
			default: break;
		}
	}

	virtual void handle (const char**) { /* empty */ }

	virtual void handle (Ast_Arguments** args) {
		for (auto &exp : (*args)->unnamed) {
			this->handle(&exp);
		}
		for (auto &exp : (*args)->named) {
			this->handle(&exp.second);
		}
	}

	virtual void handle (Ast_Scope** _block) {
		size_t initial_size = (*_block)->statements.size();
		for (uint64_t i = 0; i < (*_block)->statements.size(); i++) {
			this->handle(&((*_block)->statements[i]));

			// INFO: if we add something to the scope, we have to increment
			// the counter the same amount of items
			if ((*_block)->statements.size() != initial_size) {
				initial_size = (*_block)->statements.size();
				i -= 1;
			}
		}
	}

	virtual void handle (Ast_Declaration** decl) {
		if ((*decl)->type) this->handle(&(*decl)->type);
		if ((*decl)->expression) this->handle(&(*decl)->expression);
	}

	virtual void handle (Ast_Return** ret) {
		if ((*ret)->expression) this->handle(&(*ret)->expression);
	}

	virtual void handle (Ast_If** _if) {
		this->handle(&(*_if)->condition);
		this->handle(&(*_if)->then_scope);
		if ((*_if)->else_scope) {
			this->handle(&(*_if)->else_scope);
		}
	}

	virtual void handle (Ast_While** _while) {
		this->handle(&(*_while)->condition);
		this->handle(&(*_while)->scope);
	}

	virtual void handle (Ast_Break**) {}

	virtual void handle (Ast_Expression** exp) {
		switch ((*exp)->exp_type) {
			case AST_EXPRESSION_DIRECTIVE: {
				this->handle(reinterpret_cast<Ast_Directive**>(exp));
				break;
			}
			case AST_EXPRESSION_FUNCTION: {
				this->handle(reinterpret_cast<Ast_Function**>(exp));
				break;
			}
			case AST_EXPRESSION_CALL: {
				this->handle(reinterpret_cast<Ast_Function_Call**>(exp));
				break;
			}
			case AST_EXPRESSION_BINARY: {
				this->handle(reinterpret_cast<Ast_Binary**>(exp));
				break;
			}
			case AST_EXPRESSION_UNARY: {
				this->handle(reinterpret_cast<Ast_Unary**>(exp));
				break;
			}
			case AST_EXPRESSION_CAST: {
				this->handle(reinterpret_cast<Ast_Cast**>(exp));
				break;
			}
			case AST_EXPRESSION_IDENT: {
				this->handle(reinterpret_cast<Ast_Ident**>(exp));
				break;
			}
			case AST_EXPRESSION_LITERAL: {
				this->handle(reinterpret_cast<Ast_Literal**>(exp));
				break;
			}
			case AST_EXPRESSION_TYPE_INSTANCE: {
				this->handle(reinterpret_cast<Ast_Type_Instance**>(exp));
				break;
			}
		}
	}

	virtual void handle (Ast_Directive** directive) {
		switch ((*directive)->dir_type) {
			case AST_DIRECTIVE_IMPORT: {
				this->handle(reinterpret_cast<Ast_Directive_Import**>(directive));
				break;
			}
			case AST_DIRECTIVE_FOREIGN: {
				this->handle(reinterpret_cast<Ast_Directive_Foreign**>(directive));
				break;
			}
			case AST_DIRECTIVE_RUN: {
				this->handle(reinterpret_cast<Ast_Directive_Run**>(directive));
				break;
			}
			case AST_DIRECTIVE_IF: {
				this->handle(reinterpret_cast<Ast_Directive_If**>(directive));
				break;
			}
			default: break;
		}
	}

	virtual void handle (Ast_Directive_Import**) { /* empty */ }

	virtual void handle (Ast_Directive_Foreign**) { /* empty */ }

	virtual void handle (Ast_Directive_Run** run) {
		this->handle(&(*run)->expression);
	}

	virtual void handle (Ast_Directive_If** _if) {
		this->handle(&(*_if)->stm_if->condition);
	}

	virtual void handle (Ast_Function** func) {
		this->handle(&(*func)->type);
		if ((*func)->scope) this->handle(&(*func)->scope);
	}

	virtual void handle (Ast_Function_Call** func_call) {
		if ((*func_call)->func->exp_type != AST_EXPRESSION_FUNCTION) {
			this->handle(&(*func_call)->func);
		}
		this->handle(&(*func_call)->arguments);
	}

	virtual void handle (Ast_Binary** binary) {
		this->handle(&(*binary)->lhs);
		this->handle(&(*binary)->rhs);
	}

	virtual void handle (Ast_Unary** unary) {
		this->handle(&(*unary)->exp);
	}

	virtual void handle (Ast_Cast** cast) {
		this->handle(&(*cast)->value);
		this->handle(&(*cast)->cast_to);
	}

	virtual void handle (Ast_Ident**) {}
	virtual void handle (Ast_Literal**) {}

	virtual void handle (Ast_Type_Instance** type_def) {
		switch ((*type_def)->typedef_type) {
			case AST_TYPEDEF_STRUCT: {
				this->handle(reinterpret_cast<Ast_Struct_Type**>(type_def));
				break;
			}
			case AST_TYPEDEF_POINTER: {
				this->handle(reinterpret_cast<Ast_Pointer_Type**>(type_def));
				break;
			}
			case AST_TYPEDEF_FUNCTION: {
				this->handle(reinterpret_cast<Ast_Function_Type**>(type_def));
				break;
			}
			case AST_TYPEDEF_ARRAY: {
				this->handle(reinterpret_cast<Ast_Array_Type**>(type_def));
				break;
			}
		}
	}

	virtual void handle (Ast_Struct_Type** _struct) {
		for (auto &attr : (*_struct)->attributes) {
			this->handle(&attr);
		}
	}

	virtual void handle (Ast_Pointer_Type** _ptr) {
		this->handle(&(*_ptr)->base);
	}

	virtual void handle (Ast_Function_Type** func_type) {
		for (auto &arg_type : (*func_type)->arg_decls) {
			this->handle(&arg_type);
		}
		this->handle(&(*func_type)->ret_type);
	}

	virtual void handle (Ast_Array_Type** arr) {
		this->handle(&(*arr)->base);
		if ((*arr)->length) this->handle(&(*arr)->length);
	}
};
