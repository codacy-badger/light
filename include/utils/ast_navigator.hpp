#pragma once

#include "ast/nodes.hpp"

#include <vector>

struct Ast_Navigator {
    Ast_Scope* current_scope = NULL;

    virtual void ast_handle (Ast_Statement* stm) {
		switch (stm->stm_type) {
			case AST_STATEMENT_SCOPE: {
				this->ast_handle(reinterpret_cast<Ast_Scope*>(stm));
				break;
			}
			case AST_STATEMENT_ASSIGN: {
				this->ast_handle(reinterpret_cast<Ast_Assign*>(stm));
				break;
			}
			case AST_STATEMENT_IF: {
				this->ast_handle(reinterpret_cast<Ast_If*>(stm));
				break;
			}
			case AST_STATEMENT_WHILE: {
				this->ast_handle(reinterpret_cast<Ast_While*>(stm));
				break;
			}
			case AST_STATEMENT_DECLARATION: {
				this->ast_handle(reinterpret_cast<Ast_Declaration*>(stm));
				break;
			}
			case AST_STATEMENT_RETURN: {
				this->ast_handle(reinterpret_cast<Ast_Return*>(stm));
				break;
			}
			case AST_STATEMENT_BREAK: {
				this->ast_handle(reinterpret_cast<Ast_Break*>(stm));
				break;
			}
			case AST_STATEMENT_FOREIGN: {
				this->ast_handle(reinterpret_cast<Ast_Foreign*>(stm));
				break;
			}
			case AST_STATEMENT_STATIC_IF: {
				this->ast_handle(reinterpret_cast<Ast_Static_If*>(stm));
				break;
			}
			case AST_STATEMENT_EXPRESSION: {
				this->ast_handle(reinterpret_cast<Ast_Expression*>(stm));
				break;
			}
			default: break;
		}
	}

	virtual void ast_handle (Ast_Arguments* args) {
		for (auto exp : args->unnamed) {
			this->ast_handle(exp);
		}
		for (auto exp : args->named) {
			this->ast_handle(exp.second);
		}
	}

	virtual void ast_handle (Ast_Scope* scope) {
        auto tmp = this->current_scope;
        this->current_scope = scope;

        auto initial_size = scope->statements.size();
		for (uint64_t i = 0; i < scope->statements.size();) {
            auto stm = scope->statements[i];
			this->ast_handle(stm);

            if (initial_size != scope->statements.size()) {
                initial_size = scope->statements.size();
            } else i++;
		}

        this->current_scope = tmp;
	}

	virtual void ast_handle (Ast_Assign* assign) {
        this->ast_handle(assign->variable);
        this->ast_handle(assign->value);
	}

	virtual void ast_handle (Ast_Declaration* decl) {
		if (decl->type) this->ast_handle(decl->type);
		if (decl->expression) this->ast_handle(decl->expression);
	}

	virtual void ast_handle (Ast_Return* ret) {
		if (ret->expression) this->ast_handle(ret->expression);
	}

	virtual void ast_handle (Ast_If* _if) {
		this->ast_handle(_if->condition);
		this->ast_handle(_if->then_body);
		if (_if->else_body) {
			this->ast_handle(_if->else_body);
		}
	}

	virtual void ast_handle (Ast_While* _while) {
		this->ast_handle(_while->condition);
		this->ast_handle(_while->body);
	}

	virtual void ast_handle (Ast_Import*) { /* empty */ }

	virtual void ast_handle (Ast_Foreign*) { /* empty */ }

	virtual void ast_handle (Ast_Static_If* _if) {
		this->ast_handle(_if->stm_if->condition);
	}

	virtual void ast_handle (Ast_Break*) { /* empty */ }

	virtual void ast_handle (Ast_Expression* exp) {
		switch (exp->exp_type) {
			case AST_EXPRESSION_RUN: {
				this->ast_handle(reinterpret_cast<Ast_Run*>(exp));
				break;
			}
			case AST_EXPRESSION_IMPORT: {
				this->ast_handle(reinterpret_cast<Ast_Import*>(exp));
				break;
			}
			case AST_EXPRESSION_FUNCTION: {
				this->ast_handle(reinterpret_cast<Ast_Function*>(exp));
				break;
			}
			case AST_EXPRESSION_CALL: {
				this->ast_handle(reinterpret_cast<Ast_Function_Call*>(exp));
				break;
			}
			case AST_EXPRESSION_BINARY: {
				this->ast_handle(reinterpret_cast<Ast_Binary*>(exp));
				break;
			}
			case AST_EXPRESSION_UNARY: {
				this->ast_handle(reinterpret_cast<Ast_Unary*>(exp));
				break;
			}
			case AST_EXPRESSION_CAST: {
				this->ast_handle(reinterpret_cast<Ast_Cast*>(exp));
				break;
			}
			case AST_EXPRESSION_IDENT: {
				this->ast_handle(reinterpret_cast<Ast_Ident*>(exp));
				break;
			}
			case AST_EXPRESSION_LITERAL: {
				this->ast_handle(reinterpret_cast<Ast_Literal*>(exp));
				break;
			}
			case AST_EXPRESSION_TYPE: {
				this->ast_handle(reinterpret_cast<Ast_Type*>(exp));
				break;
			}
		}
	}

	virtual void ast_handle (Ast_Run* run) {
		this->ast_handle(run->expression);
	}

	virtual void ast_handle (Ast_Function* func) {
        if (func->func_flags & FUNCTION_FLAG_BEING_CHECKED) return;

        func->func_flags |= FUNCTION_FLAG_BEING_CHECKED;
		this->ast_handle(func->type);
		if (func->body) {
            this->ast_handle(func->body);
        }
        func->func_flags &= ~FUNCTION_FLAG_BEING_CHECKED;
	}

	virtual void ast_handle (Ast_Function_Call* func_call) {
		if (func_call->func->exp_type != AST_EXPRESSION_FUNCTION) {
			this->ast_handle(func_call->func);
		}
		this->ast_handle(func_call->arguments);
	}

	virtual void ast_handle (Ast_Binary* binary) {
		this->ast_handle(binary->lhs);
		this->ast_handle(binary->rhs);
	}

	virtual void ast_handle (Ast_Unary* unary) {
		this->ast_handle(unary->exp);
	}

	virtual void ast_handle (Ast_Cast* cast) {
		this->ast_handle(cast->value);
		this->ast_handle(cast->cast_to);
	}

	virtual void ast_handle (Ast_Ident*) { /* empty */ }

	virtual void ast_handle (Ast_Literal*) { /* empty */ }

	virtual void ast_handle (Ast_Type* type_def) {
		switch (type_def->typedef_type) {
			case AST_TYPEDEF_STRUCT: {
				this->ast_handle(reinterpret_cast<Ast_Struct_Type*>(type_def));
				break;
			}
			case AST_TYPEDEF_POINTER: {
				this->ast_handle(reinterpret_cast<Ast_Pointer_Type*>(type_def));
				break;
			}
			case AST_TYPEDEF_FUNCTION: {
				this->ast_handle(reinterpret_cast<Ast_Function_Type*>(type_def));
				break;
			}
			case AST_TYPEDEF_ARRAY: {
				this->ast_handle(reinterpret_cast<Ast_Array_Type*>(type_def));
				break;
			}
		}
	}

	virtual void ast_handle (Ast_Struct_Type* _struct) {
        if (_struct->struct_flags & STRUCT_FLAG_BEING_CHECKED) return;

        _struct->struct_flags |= STRUCT_FLAG_BEING_CHECKED;
        this->ast_handle(&_struct->scope);
        _struct->struct_flags &= ~STRUCT_FLAG_BEING_CHECKED;
	}

	virtual void ast_handle (Ast_Pointer_Type* _ptr) {
		this->ast_handle(_ptr->base);
	}

	virtual void ast_handle (Ast_Function_Type* func_type) {
		for (auto arg_type : func_type->arg_decls) {
			this->ast_handle((Ast_Statement*) arg_type);
		}
		this->ast_handle(func_type->ret_type);
	}

	virtual void ast_handle (Ast_Array_Type* arr) {
		this->ast_handle(arr->base);
		this->ast_handle(arr->length);
	}
};
