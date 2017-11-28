#pragma once

#include "parser/pipe/Type_Checking.hpp"

#include "compiler.hpp"

void Type_Checking::on_statement(Ast_Statement* stm) {
    this->check_type(stm);
    this->to_next(stm);
}

void Type_Checking::check_type (Ast_Statement* stm) {
    switch (stm->stm_type) {
        case AST_STATEMENT_BLOCK:
            check_type(static_cast<Ast_Block*>(stm));
            break;
        case AST_STATEMENT_IF:
            check_type(static_cast<Ast_If*>(stm));
            break;
        case AST_STATEMENT_WHILE:
            check_type(static_cast<Ast_While*>(stm));
            break;
        case AST_STATEMENT_DECLARATION:
            check_type(static_cast<Ast_Declaration*>(stm));
            break;
        case AST_STATEMENT_RETURN:
            check_type(static_cast<Ast_Return*>(stm));
            break;
        case AST_STATEMENT_EXPRESSION:
            check_type(static_cast<Ast_Expression*>(stm));
            break;
        default: break;
    }
}

void Type_Checking::check_type (Ast_Block* block) {
    for (auto stm : block->list) check_type(stm);
}

void Type_Checking::check_type (Ast_If* _if) {
	check_type(_if->condition);
	check_type(_if->then_statement);
	if (_if->else_statement) check_type(_if->else_statement);
}

void Type_Checking::check_type (Ast_While* _while) {
	check_type(_while->condition);
	check_type(_while->statement);
}

void Type_Checking::check_type (Ast_Declaration* decl) {
	if (decl->type) check_type(decl->type);

	if (decl->expression) {
		check_type(decl->expression);
		if (!decl->expression->inferred_type) {
			Light_Compiler::inst->error_stop(decl->expression, "Expression type could not be inferred!");
		}

		if (decl->decl_flags & DECL_FLAG_CONSTANT) {
			// TODO: ensure expression is constant
		}

		if (!decl->type) {
			decl->type = decl->expression->inferred_type;
		}
	} else {
		if (!decl->type) {
			Light_Compiler::inst->error_stop(decl, "Cannot infer type without an expression!");
		}
	}

	if (decl->type->inferred_type != Light_Compiler::inst->type_def_type) {
		Light_Compiler::inst->error_stop(decl->type, "Expression is not a type!");
	}
}

void Type_Checking::check_type (Ast_Return* ret) {
	if (ret->exp) check_type(ret->exp);

	auto fn = ret->block->find_function();
	if (!fn) {
		Light_Compiler::inst->error_stop(ret, "Return statement must be inside a function!");
	} else if (ret->exp) {
		if (fn->type->return_type == Light_Compiler::inst->type_def_void)
			Light_Compiler::inst->error_stop(ret, "Return statment has expression, but function returns void!");
		else if (ret->exp->inferred_type != fn->type->return_type) {
			Light_Compiler::inst->error_stop(ret, "Type mismatch, return expression is '---', but function expects '---'!");
		}
	} else {
		if (fn->type->return_type != Light_Compiler::inst->type_def_void)
			Light_Compiler::inst->error_stop(ret, "Return statment has no expression, but function returns '---'!");
	}
}

void Type_Checking::check_type (Ast_Expression* exp) {
    switch (exp->exp_type) {
		case AST_EXPRESSION_TYPE_DEFINITION:
            check_type(static_cast<Ast_Type_Definition*>(exp));
            break;
        case AST_EXPRESSION_FUNCTION:
            check_type(static_cast<Ast_Function*>(exp));
            break;
        case AST_EXPRESSION_CALL:
            check_type(static_cast<Ast_Function_Call*>(exp));
            break;
        case AST_EXPRESSION_CAST:
            check_type(static_cast<Ast_Cast*>(exp));
            break;
        case AST_EXPRESSION_POINTER:
            check_type(static_cast<Ast_Pointer*>(exp));
            break;
        case AST_EXPRESSION_BINARY:
            check_type(static_cast<Ast_Binary*>(exp));
            break;
        case AST_EXPRESSION_UNARY:
            check_type(static_cast<Ast_Unary*>(exp));
            break;
        case AST_EXPRESSION_IDENT:
            check_type(static_cast<Ast_Ident*>(exp));
            break;
        case AST_EXPRESSION_LITERAL:
            check_type(static_cast<Ast_Literal*>(exp));
            break;
        default: break;
    }
}

void Type_Checking::check_type (Ast_Cast* cast) {
	check_type(cast->value);
	check_type(cast->cast_to);
	if (cast->cast_to->exp_type == AST_EXPRESSION_TYPE_DEFINITION) {
		cast->inferred_type = static_cast<Ast_Type_Definition*>(cast->cast_to);
	}
}

void Type_Checking::check_type (Ast_Pointer* ptr) {
	check_type(ptr->base);
    auto ptr_type = new Ast_Pointer_Type();
    ptr_type->base = ptr->base->inferred_type;
    ptr->inferred_type = ptr_type;
}

void Type_Checking::check_type (Ast_Type_Definition* tydef) {
    switch (tydef->typedef_type) {
		case AST_TYPEDEF_FUNCTION:
            check_type(static_cast<Ast_Function_Type*>(tydef));
            break;
        case AST_TYPEDEF_STRUCT:
            check_type(static_cast<Ast_Struct_Type*>(tydef));
            break;
        case AST_TYPEDEF_POINTER:
            check_type(static_cast<Ast_Pointer_Type*>(tydef));
            break;
        default: break;
    }
}

void Type_Checking::check_type (Ast_Function_Type* ty) {
    ty->inferred_type = Light_Compiler::inst->type_def_type;

	check_type(ty->return_type);
	for (int i = 0; i < ty->parameter_decls.size(); i++) {
		check_type(ty->parameter_decls[i]);
	}
}

void compute_struct_size (Ast_Struct_Type* _struct) {
	if (_struct->byte_size == 0) {
		uint16_t size = 0;
		for (int i = 0; i < _struct->attributes.size(); i++) {
			auto decl = _struct->attributes[i];
			decl->struct_byte_offset = size;
			decl->struct_index = i;

			assert (decl->type->exp_type == AST_EXPRESSION_TYPE_DEFINITION);
			auto defn_ty = static_cast<Ast_Type_Definition*>(decl->type);
			size += defn_ty->byte_size;
		}
		_struct->byte_size = size;
	}
}

void Type_Checking::check_type (Ast_Struct_Type* _struct) {
    _struct->inferred_type = Light_Compiler::inst->type_def_type;
	for (auto decl : _struct->attributes) {
		check_type(decl);
	}
    compute_struct_size(_struct);
}

void Type_Checking::check_type (Ast_Pointer_Type* ty) {
    ty->inferred_type = Light_Compiler::inst->type_def_type;
	check_type(ty->base);
	if (ty->base->exp_type == AST_EXPRESSION_IDENT) {
		auto ident = static_cast<Ast_Ident*>(ty->base);
		delete ty->base;
		ty->base = ident->declaration->expression;
	}
}

void Type_Checking::check_type (Ast_Function* func) {
    func->inferred_type = func->type;
	check_type(func->type);
	check_type(func->scope);
}

void Type_Checking::check_type (Ast_Function_Call* call) {
    if (call->fn->exp_type != AST_EXPRESSION_FUNCTION)
		Light_Compiler::inst->error_stop(call, "Function calls can only be performed in functions types!");

	auto func = static_cast<Ast_Function*>(call->fn);
	auto ret_ty = static_cast<Ast_Type_Definition*>(func->type->return_type);
	call->inferred_type = ret_ty;

	if (call->args) {
		for (auto exp : call->args->values)
			check_type(exp);
	}
}

void Type_Checking::check_type (Ast_Binary* binop) {
	check_type(binop->lhs);
	check_type(binop->rhs);
	if (binop->lhs->inferred_type != binop->rhs->inferred_type) {
		Light_Compiler::inst->error_stop(binop, "Type mismatch on binary expression");
	}
	switch (binop->binary_op) {
		case AST_BINARY_EQ:
		case AST_BINARY_NEQ:
		case AST_BINARY_LT:
		case AST_BINARY_LTE:
		case AST_BINARY_GT:
		case AST_BINARY_GTE: {
			binop->inferred_type = Light_Compiler::inst->type_def_bool;
			break;
		}
		case AST_BINARY_ASSIGN:
		case AST_BINARY_ADD:
		case AST_BINARY_SUB:
		case AST_BINARY_MUL:
		case AST_BINARY_DIV: {
			binop->inferred_type = binop->lhs->inferred_type;
			break;
		}
	}
}

void Type_Checking::check_type (Ast_Unary* unop) {
	check_type(unop->exp);
	switch (unop->unary_op) {
		case AST_UNARY_NEGATE: {
			auto inst = Light_Compiler::inst;
			if (unop->exp->inferred_type == inst->type_def_u8) {
				unop->inferred_type = inst->type_def_s16;
			} else if (unop->exp->inferred_type == inst->type_def_u16) {
				unop->inferred_type = inst->type_def_s32;
			} else if (unop->exp->inferred_type == inst->type_def_u32) {
				unop->inferred_type = inst->type_def_s64;
			} else if (unop->exp->inferred_type == inst->type_def_u64) {
				unop->inferred_type = inst->type_def_s64;
			} else {
				unop->inferred_type = unop->exp->inferred_type;
			}
            break;
		}
		case AST_UNARY_REFERENCE: {
            auto ptr_to = new Ast_Pointer_Type();
            ptr_to->base = unop->exp->inferred_type;
            unop->inferred_type = ptr_to;
            break;
        }
		case AST_UNARY_DEREFERENCE: {
            auto inf_type = unop->exp->inferred_type;
            if (inf_type->typedef_type == AST_TYPEDEF_POINTER) {
                auto ptr_type = static_cast<Ast_Pointer_Type*>(inf_type);
                auto base_type = static_cast<Ast_Type_Definition*>(ptr_type->base);
                unop->inferred_type = base_type;
            } else {
                Light_Compiler::inst->error_stop(unop, "Can't dereference a non-pointer type expression!");
            }
            break;
		}
	}
}

void Type_Checking::check_type (Ast_Ident* ident) {
	if (ident->declaration) {
		ident->inferred_type = static_cast<Ast_Type_Definition*>(ident->declaration->type);
	} else {
		Light_Compiler::inst->error_stop(ident, "Ident has no declaration!");
	}
}

void Type_Checking::check_type (Ast_Literal* lit) {
    //TODO: smarter inference: use the smallest possible type
    if (!lit->inferred_type) {
        switch (lit->literal_type) {
            case AST_LITERAL_UNSIGNED_INT: {
				lit->inferred_type = Light_Compiler::inst->type_def_u32;
                break;
            }
            case AST_LITERAL_SIGNED_INT: {
				lit->inferred_type = Light_Compiler::inst->type_def_s32;
                break;
            }
            case AST_LITERAL_DECIMAL: {
				lit->inferred_type = Light_Compiler::inst->type_def_f32;
                break;
            }
            case AST_LITERAL_STRING: {
				lit->inferred_type = Light_Compiler::inst->type_def_string;
                break;
            }
            default: break;
        }
    }
}
