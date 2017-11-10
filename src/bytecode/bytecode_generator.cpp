#include "bytecode/bytecode_generator.hpp"

#include "compiler.hpp"

void Bytecode_Generator::on_statement (Ast_Statement* stm) {
    this->gen(stm);
    this->to_next(stm);
}

void Bytecode_Generator::gen (Ast_Statement* stm, vector<Instruction*>* bytecode, size_t reg) {
    switch (stm->stm_type) {
        case AST_STATEMENT_DECLARATION: {
            this->gen(static_cast<Ast_Declaration*>(stm));
            break;
        }
        default: return;
    }
}

void Bytecode_Generator::gen (Ast_Block* block, vector<Instruction*>* bytecode, size_t reg) {
    for (auto stm : block->list) {
		this->gen(stm, bytecode);
	};
}

void Bytecode_Generator::gen (Ast_Declaration* decl, vector<Instruction*>* bytecode, size_t reg) {
    if (decl->decl_flags & DECL_FLAG_CONSTANT) {
		if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
			this->stack_offset = 0;
			this->gen(static_cast<Ast_Function*>(decl->expression));
		} else {
			if (decl->expression->exp_type == AST_EXPRESSION_TYPE_DEFINITION) {
				return;
			} else if (decl->expression->exp_type == AST_EXPRESSION_LITERAL) {
				size_t const_offset = 0;
				//auto interp = Light_Compiler::instance->interp;
				auto lit = static_cast<Ast_Literal*>(decl->expression);
				switch (lit->literal_type) {
					case AST_LITERAL_SIGNED_INT: {
						// TODO: switch over inferred_type to store exact size
						//const_offset = interp->constant->store(lit->int_value);
						break;
					}
					case AST_LITERAL_UNSIGNED_INT: {
						// TODO: switch over inferred_type to store exact size
						//const_offset = interp->constant->store(lit->uint_value);
						break;
					}
					case AST_LITERAL_DECIMAL: {
						// TODO: switch over inferred_type to store exact size
						//const_offset = interp->constant->store(lit->decimal_value);
						break;
					}
					case AST_LITERAL_STRING: {
						//const_offset = interp->constant->store(lit->string_value);
						break;
					}
				}
				decl->data_offset = const_offset;
			} else {
				//Light_Compiler::instance->error_stop(decl, "Only literal values supported on constant declarations!");
			}
		}
    } else {
		printf("Ast_Declaration '%s'\n", decl->name);
		auto ty_decl = static_cast<Ast_Type_Definition*>(decl->type);
		if (decl->scope->is_global()) {
			// TODO: reserve space in the global storage
			// auto offset = Light_Compiler::instance->interp->global->reserve(decl->type->byte_size);
			// decl->data_offset = offset;

			if (decl->expression) {
				auto _reg = this->gen(decl->expression);
				printf("\tBYTECODE_GLOBAL_OFFSET %zd, %lld\n", _reg + 1, decl->data_offset);
				printf("\tBYTECODE_STORE %zd, %zd, %lld\n", _reg + 1, _reg, ty_decl->byte_size);
			}
		} else {
			printf("\tBYTECODE_STACK_ALLOCATE %zd\n", ty_decl->byte_size);
			decl->data_offset = this->stack_offset;
			this->stack_offset += ty_decl->byte_size;
			printf("\t; Stack size after alloca %zd\n", this->stack_offset);
			if (decl->expression) {
				auto _reg = this->gen(decl->expression);
				printf("\tBYTECODE_STACK_OFFSET %zd, %lld\n", _reg + 1, decl->data_offset);
				printf("\tBYTECODE_STORE %zd, %zd, %lld\n", _reg + 1, _reg, ty_decl->byte_size);
			} else printf("\t; No value to store\n");
		}
	}
}

size_t Bytecode_Generator::gen (Ast_Expression* exp, vector<Instruction*>* bytecode, size_t reg) {
    switch (exp->exp_type) {
        case AST_EXPRESSION_LITERAL: return this->gen(static_cast<Ast_Literal*>(exp), bytecode, reg);
		case AST_EXPRESSION_UNARY: return this->gen(static_cast<Ast_Unary*>(exp), bytecode, reg);
        case AST_EXPRESSION_BINARY: return this->gen(static_cast<Ast_Binary*>(exp), bytecode, reg);
        case AST_EXPRESSION_IDENT: return this->gen(static_cast<Ast_Ident*>(exp), bytecode, reg);
        case AST_EXPRESSION_FUNCTION: return this->gen(static_cast<Ast_Function*>(exp), bytecode, reg);
        default: return reg;
    }
}

size_t Bytecode_Generator::gen (Ast_Literal* lit, vector<Instruction*>* bytecode, size_t reg) {
	switch (lit->literal_type) {
		case AST_LITERAL_SIGNED_INT: {
			printf("\tBYTECODE_SET_INTEGER %zd, 8, %lld\n", reg, lit->int_value);
			return reg;
		}
		case AST_LITERAL_UNSIGNED_INT: {
			printf("\tBYTECODE_SET_INTEGER %zd, 8, %lld\n", reg, lit->uint_value);
			return reg;
		}
		case AST_LITERAL_DECIMAL: {
			printf("\tBYTECODE_SET_DECIMAL %zd, 8, 0x%llf\n", reg, lit->decimal_value);
			return reg;
		}
		default: {
			Light_Compiler::instance->error_stop(lit, "Literal type to bytecode conversion not supported!");
			return reg;
		}
	}
}

size_t Bytecode_Generator::gen (Ast_Unary* unop, vector<Instruction*>* bytecode, size_t reg) {
	size_t next_reg = this->gen(unop->exp);
	switch (unop->unary_op) {
		case AST_UNARY_NEGATE: {
			printf("\tBYTECODE_NEG %zd\n", next_reg);
			return reg;
		}
		case AST_UNARY_NOT: {
			printf("\tBYTECODE_NOT %zd\n", next_reg);
			return reg;
		}
		default: return reg;
	}
}

size_t Bytecode_Generator::gen (Ast_Binary* binop, vector<Instruction*>* bytecode, size_t reg) {
	size_t rhs_reg = this->gen(binop->rhs, bytecode, reg);
	size_t lhs_reg = this->gen(binop->lhs, bytecode, rhs_reg + 1);
	switch (binop->binary_op) {
		case AST_BINARY_ADD: {
			printf("\tBYTECODE_ADD %zd, %zd\n", rhs_reg, lhs_reg);
			return reg;
		}
		case AST_BINARY_SUB: {
			printf("\tBYTECODE_SUB %zd, %zd\n", rhs_reg, lhs_reg);
			return reg;
		}
		default: return reg;
	}
}

size_t Bytecode_Generator::gen (Ast_Ident* ident, vector<Instruction*>* bytecode, size_t reg) {
	if (ident->declaration->is_global()) {
		if (ident->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
			printf("\tBYTECODE_LOAD_GLOBAL (%s) %zd, %zd, %zd\n", ident->name, reg, ident->declaration->data_offset, ident->inferred_type->byte_size);
		} else {
			printf("\tBYTECODE_LOAD_GLOBAL_POINTER (%s) %zd, %zd\n", ident->name, reg, ident->declaration->data_offset);
		}
		return reg;
	} else {
		printf("\t; Load ident '%s' from [stack] @ %zd\n", ident->name, ident->declaration->data_offset);
		if (ident->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
			printf("\tBYTECODE_STACK_OFFSET %zd, %zd\n", reg + 1, ident->declaration->data_offset);
			printf("\tBYTECODE_DEREFERENCE %zd, %zd, %zd\n", reg, reg + 1, ident->inferred_type->byte_size);
		} else {
			printf("\tBYTECODE_STACK_OFFSET %zd, %zd\n", reg, ident->declaration->data_offset);
		}
		return reg;
	}
}

size_t Bytecode_Generator::gen (Ast_Function* fn, vector<Instruction*>* bytecode, size_t reg) {
	if (fn->scope) {
		this->gen(fn->scope, &fn->bytecode);
	}

	return reg;
}