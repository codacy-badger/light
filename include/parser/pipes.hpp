#pragma once

#include <assert.h>
#include "parser/ast.hpp"
#include "timer.hpp"

#define PIPE_NAME(name) name() { this->pipe_name = #name; }

struct Pipe {

	double accumulated_spans = 0;
	const char* pipe_name = NULL;
	Pipe* next = NULL;

	virtual void on_statement (Ast_Statement** stm) {
		auto start = Timer::getTime();
		this->handle(stm);
		this->accumulated_spans += Timer::clockStop(start);
		this->to_next(stm);
	}

	virtual void on_finish (double full_time) {
		if (this->pipe_name) {
			assert(full_time > 0);
			double percent = (this->accumulated_spans * 100.0) / full_time;
			printf("  - %-25s%8.6fs ( %5.2f %% )\n", this->pipe_name, this->accumulated_spans, percent);
		}
		this->try_finish(full_time);
	}

	void to_next (Ast_Statement** stm) {
		if (next) next->on_statement(stm);
	}

	void try_finish(double full_time) {
		if (next) next->on_finish(full_time);
	}

	void append (Pipe* next_pipe) {
		Pipe* last = this;
		while (last->next) last = last->next;
		last->next = next_pipe;
	}

	void handle (Ast_Statement** stm) {
		for (auto &note : (*stm)->notes) {
			this->handle(&note);
		}
		switch ((*stm)->stm_type) {
			case AST_STATEMENT_BLOCK: {
				this->handle(reinterpret_cast<Ast_Block**>(stm));
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
		for (auto &stm : (*_block)->list) {
			this->handle(&stm);
		}
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

	void handle (Ast_Expression** exp) {
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
			case AST_EXPRESSION_TYPE_DEFINITION: {
				this->handle(reinterpret_cast<Ast_Type_Definition**>(exp));
				break;
			}
		}

	}

	virtual void handle (Ast_Function** func) {
		for (auto &arg : (*func)->arg_decls) {
			this->handle(&arg);
		}
		if ((*func)->scope) this->handle(&(*func)->scope);
	}

	virtual void handle (Ast_Function_Call** func_call) {
		if ((*func_call)->fn->exp_type != AST_EXPRESSION_FUNCTION) {
			this->handle(&(*func_call)->fn);
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

	void handle (Ast_Type_Definition** type_def) {
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
		for (auto &arg_type : (*func_type)->arg_types) {
			this->handle(&arg_type);
		}
		this->handle(&(*func_type)->ret_type);
	}

	virtual void handle (Ast_Array_Type** arr) {
		this->handle(&(*arr)->base);
		if ((*arr)->length_exp) this->handle(&(*arr)->length_exp);
	}

	virtual void handle (Ast_Slice_Type** slice) {
		this->handle(&(*slice)->base);
	}
};
