#pragma once

#include "ast/ast.hpp"

struct Ast_Ref_Navigator {
    virtual void ast_handle (Ast_Statement** stm) {
		for (auto &note : (*stm)->notes) {
			this->ast_handle(&note);
		}
		switch ((*stm)->stm_type) {
			case AST_STATEMENT_SCOPE: {
				this->ast_handle(reinterpret_cast<Ast_Scope**>(stm));
				break;
			}
			case AST_STATEMENT_IF: {
				this->ast_handle(reinterpret_cast<Ast_If**>(stm));
				break;
			}
			case AST_STATEMENT_WHILE: {
				this->ast_handle(reinterpret_cast<Ast_While**>(stm));
				break;
			}
			case AST_STATEMENT_DECLARATION: {
				this->ast_handle(reinterpret_cast<Ast_Declaration**>(stm));
				break;
			}
			case AST_STATEMENT_RETURN: {
				this->ast_handle(reinterpret_cast<Ast_Return**>(stm));
				break;
			}
			case AST_STATEMENT_BREAK: {
				this->ast_handle(reinterpret_cast<Ast_Break**>(stm));
				break;
			}
			case AST_STATEMENT_IMPORT: {
				this->ast_handle(reinterpret_cast<Ast_Import**>(stm));
				break;
			}
			case AST_STATEMENT_FOREIGN: {
				this->ast_handle(reinterpret_cast<Ast_Foreign**>(stm));
				break;
			}
			case AST_STATEMENT_STATIC_IF: {
				this->ast_handle(reinterpret_cast<Ast_Static_If**>(stm));
				break;
			}
			case AST_STATEMENT_EXPRESSION: {
				this->ast_handle(reinterpret_cast<Ast_Expression**>(stm));
				break;
			}
			default: break;
		}
	}

	virtual void ast_handle (const char**) { /* empty */ }

	virtual void ast_handle (Ast_Arguments** args) {
		for (auto &exp : (*args)->unnamed) {
			this->ast_handle(&exp);
		}
		for (auto &exp : (*args)->named) {
			this->ast_handle(&exp.second);
		}
	}

	virtual void ast_handle (Ast_Scope** _block) {
		size_t initial_size = (*_block)->statements.size();
		for (uint64_t i = 0; i < (*_block)->statements.size(); i++) {
			this->ast_handle(&((*_block)->statements[i]));

			// INFO: if we add something to the scope, we have to increment
			// the counter the same amount of items
			if ((*_block)->statements.size() != initial_size) {
				initial_size = (*_block)->statements.size();
				i -= 1;
			}
		}
	}

	virtual void ast_handle (Ast_Declaration** decl) {
		if ((*decl)->type) this->ast_handle(&(*decl)->type);
		if ((*decl)->expression) this->ast_handle(&(*decl)->expression);
	}

	virtual void ast_handle (Ast_Return** ret) {
		if ((*ret)->expression) this->ast_handle(&(*ret)->expression);
	}

	virtual void ast_handle (Ast_If** _if) {
		this->ast_handle(&(*_if)->condition);
		this->ast_handle(&(*_if)->then_scope);
		if ((*_if)->else_scope) {
			this->ast_handle(&(*_if)->else_scope);
		}
	}

	virtual void ast_handle (Ast_While** _while) {
		this->ast_handle(&(*_while)->condition);
		this->ast_handle(&(*_while)->scope);
	}

	virtual void ast_handle (Ast_Static_If** _if) {
		this->ast_handle(&(*_if)->stm_if->condition);
	}

	virtual void ast_handle (Ast_Import**) { /* empty */ }

	virtual void ast_handle (Ast_Foreign**) { /* empty */ }

	virtual void ast_handle (Ast_Break**) {}

	virtual void ast_handle (Ast_Expression** exp) {
		switch ((*exp)->exp_type) {
			case AST_EXPRESSION_RUN: {
				this->ast_handle(reinterpret_cast<Ast_Run**>(exp));
				break;
			}
			case AST_EXPRESSION_FUNCTION: {
				this->ast_handle(reinterpret_cast<Ast_Function**>(exp));
				break;
			}
			case AST_EXPRESSION_CALL: {
				this->ast_handle(reinterpret_cast<Ast_Function_Call**>(exp));
				break;
			}
			case AST_EXPRESSION_BINARY: {
				this->ast_handle(reinterpret_cast<Ast_Binary**>(exp));
				break;
			}
			case AST_EXPRESSION_UNARY: {
				this->ast_handle(reinterpret_cast<Ast_Unary**>(exp));
				break;
			}
			case AST_EXPRESSION_CAST: {
				this->ast_handle(reinterpret_cast<Ast_Cast**>(exp));
				break;
			}
			case AST_EXPRESSION_IDENT: {
				this->ast_handle(reinterpret_cast<Ast_Ident**>(exp));
				break;
			}
			case AST_EXPRESSION_LITERAL: {
				this->ast_handle(reinterpret_cast<Ast_Literal**>(exp));
				break;
			}
			case AST_EXPRESSION_TYPE_INSTANCE: {
				this->ast_handle(reinterpret_cast<Ast_Type_Instance**>(exp));
				break;
			}
		}
	}

	virtual void ast_handle (Ast_Run** run) {
		this->ast_handle(&(*run)->expression);
	}

	virtual void ast_handle (Ast_Function** func) {
		this->ast_handle(&(*func)->type);
		if ((*func)->scope) this->ast_handle(&(*func)->scope);
	}

	virtual void ast_handle (Ast_Function_Call** func_call) {
		if ((*func_call)->func->exp_type != AST_EXPRESSION_FUNCTION) {
			this->ast_handle(&(*func_call)->func);
		}
		this->ast_handle(&(*func_call)->arguments);
	}

	virtual void ast_handle (Ast_Binary** binary) {
		this->ast_handle(&(*binary)->lhs);
		this->ast_handle(&(*binary)->rhs);
	}

	virtual void ast_handle (Ast_Unary** unary) {
		this->ast_handle(&(*unary)->exp);
	}

	virtual void ast_handle (Ast_Cast** cast) {
		this->ast_handle(&(*cast)->value);
		this->ast_handle(&(*cast)->cast_to);
	}

	virtual void ast_handle (Ast_Ident**) {}
	virtual void ast_handle (Ast_Literal**) {}

	virtual void ast_handle (Ast_Type_Instance** type_def) {
		switch ((*type_def)->typedef_type) {
			case AST_TYPEDEF_STRUCT: {
				this->ast_handle(reinterpret_cast<Ast_Struct_Type**>(type_def));
				break;
			}
			case AST_TYPEDEF_POINTER: {
				this->ast_handle(reinterpret_cast<Ast_Pointer_Type**>(type_def));
				break;
			}
			case AST_TYPEDEF_FUNCTION: {
				this->ast_handle(reinterpret_cast<Ast_Function_Type**>(type_def));
				break;
			}
			case AST_TYPEDEF_ARRAY: {
				this->ast_handle(reinterpret_cast<Ast_Array_Type**>(type_def));
				break;
			}
		}
	}

	virtual void ast_handle (Ast_Struct_Type** _struct) {
		for (auto &attr : (*_struct)->attributes) {
			this->ast_handle(&attr);
		}
	}

	virtual void ast_handle (Ast_Pointer_Type** _ptr) {
		this->ast_handle(&(*_ptr)->base);
	}

	virtual void ast_handle (Ast_Function_Type** func_type) {
		for (auto &arg_type : (*func_type)->arg_decls) {
			this->ast_handle(&arg_type);
		}
		this->ast_handle(&(*func_type)->ret_type);
	}

	virtual void ast_handle (Ast_Array_Type** arr) {
		this->ast_handle(&(*arr)->base);
		if ((*arr)->length) this->ast_handle(&(*arr)->length);
	}
};
