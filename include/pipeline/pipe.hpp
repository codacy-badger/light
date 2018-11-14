#pragma once

#include "parser/ast.hpp"
#include "platform.hpp"

#include <stdint.h>

#define PIPE_NAME(name) name() { this->pipe_name = #name; }

#define PRINT_METRIC(format, ...) printf("        " format "\n", __VA_ARGS__)

struct Pipe {
	const char* pipe_name = NULL;
	double total_time = 0;

	bool stop_processing = false;
	deque<Ast_Statement*> pending_stms;

	size_t priority = 0;

	virtual void on_statement (Ast_Statement** stm) {
		auto start = os_get_user_time();
		this->handle(stm);
		this->total_time += os_time_user_stop(start);
	}

	virtual void on_finish () { /* empty */ }

	virtual void print_pipe_metrics () { /* empty by default */ }

	virtual void handle (Ast_Statement** stm) {
		for (auto &note : (*stm)->notes) {
			this->handle(&note);
		}
		switch ((*stm)->stm_type) {
			case AST_STATEMENT_BLOCK: {
				this->handle(reinterpret_cast<Ast_Block**>(stm));
				break;
			}
			case AST_STATEMENT_IMPORT: {
				this->handle(reinterpret_cast<Ast_Import**>(stm));
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

	virtual void handle (Ast_Note** note) {
		for (auto &exp : (*note)->arguments) {
			this->handle(&exp);
		}
	}

	virtual void handle (Ast_Block** _block) {
		auto it = (*_block)->list.begin();
		while (it != (*_block)->list.end()) {
			this->handle(&(*it));
			it++;
		}
	}

	virtual void handle (Ast_Import** import) {
		this->handle(&(*import)->target);
	}

	virtual void handle (Ast_Declaration** decl) {
		if ((*decl)->type) this->handle(&(*decl)->type);
		if ((*decl)->expression) this->handle(&(*decl)->expression);
	}

	virtual void handle (Ast_Return** ret) {
		if ((*ret)->exp) this->handle(&(*ret)->exp);
	}

	virtual void handle (Ast_If** _if) {
		this->handle(&(*_if)->condition);
		this->handle(&(*_if)->then_statement);
		if ((*_if)->else_statement) {
			this->handle(&(*_if)->else_statement);
		}
	}

	virtual void handle (Ast_While** _while) {
		this->handle(&(*_while)->condition);
		this->handle(&(*_while)->statement);
	}

	virtual void handle (Ast_Break**) {}

	virtual void handle (Ast_Expression** exp) {
		switch ((*exp)->exp_type) {
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

	virtual void handle (Ast_Function** func) {
		this->handle(&(*func)->type);
		if ((*func)->scope) this->handle(&(*func)->scope);
	}

	virtual void handle (Ast_Function_Call** func_call) {
		if ((*func_call)->func->exp_type != AST_EXPRESSION_FUNCTION) {
			this->handle(&(*func_call)->func);
		}
		for (auto &exp : (*func_call)->arguments) {
			this->handle(&exp);
		}
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
		if ((*_struct)->is_slice) {
			this->handle(reinterpret_cast<Ast_Slice_Type**>(_struct));
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

	virtual void handle (Ast_Slice_Type** slice) {
		this->handle(&(*slice)->base);
	}
};
