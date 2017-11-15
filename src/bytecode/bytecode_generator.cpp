#include "bytecode/bytecode_generator.hpp"

#include "compiler.hpp"

void copy_location_info (Instruction* intruction, Ast* node) {
    intruction->filename = node->filename;
    intruction->line = node->line;
}

void Bytecode_Generator::on_statement (Ast_Statement* stm) {
    this->gen(stm, NULL, 0);
    this->to_next(stm);
}

void Bytecode_Generator::gen (Ast_Statement* stm, vector<Instruction*>* bytecode, size_t reg) {
    switch (stm->stm_type) {
        case AST_STATEMENT_DECLARATION: {
            this->gen(static_cast<Ast_Declaration*>(stm), bytecode, reg);
            break;
        }
		case AST_STATEMENT_RETURN: {
			this->gen(static_cast<Ast_Return*>(stm), bytecode, reg);
			break;
		}
        default: return;
    }
}

void Bytecode_Generator::gen (Ast_Block* block, vector<Instruction*>* bytecode, size_t reg) {
    for (auto stm : block->list) {
		this->gen(stm, bytecode, reg);
	};
}

void Bytecode_Generator::gen (Ast_Return* ret, vector<Instruction*>* bytecode, size_t reg) {
    auto ret_reg = this->gen(ret->exp, bytecode, reg);
    printf("\tBYTECODE_RETURN %zd\n", ret_reg);
}

void Bytecode_Generator::gen (Ast_Declaration* decl, vector<Instruction*>* bytecode, size_t reg) {
    if (decl->decl_flags & DECL_FLAG_CONSTANT) {
		if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
			auto _tmp = this->stack_offset;
            this->stack_offset = 0;
			this->gen(static_cast<Ast_Function*>(decl->expression), bytecode, reg);
            this->stack_offset = _tmp;
		}
    } else {
		auto ty_decl = static_cast<Ast_Type_Definition*>(decl->type);
		if (decl->scope->is_global()) {
            auto ty_defn = static_cast<Ast_Type_Definition*>(decl->type);
			decl->data_offset = this->global_offset;
            this->global_offset += ty_defn->byte_size;

			if (decl->expression) {
				auto _reg = this->gen(decl->expression, bytecode, reg);
				printf("\tBYTECODE_GLOBAL_OFFSET %zd, %lld\n", _reg + 1, decl->data_offset);
				printf("\tBYTECODE_STORE %zd, %zd, %lld\n", _reg + 1, _reg, ty_decl->byte_size);
			}
		} else {
			printf("\tBYTECODE_STACK_ALLOCATE %zd\n", ty_decl->byte_size);
            auto inst = new Inst_Stack_Allocate(ty_decl->byte_size);
            copy_location_info(inst, decl);
            bytecode->push_back(inst);

			decl->data_offset = this->stack_offset;
			this->stack_offset += ty_decl->byte_size;
			printf("\t; Stack size after alloca %zd\n", this->stack_offset);
			if (decl->expression) {
				auto _reg = this->gen(decl->expression, bytecode, reg);
				printf("\tBYTECODE_STACK_OFFSET %zd, %lld\n", _reg + 1, decl->data_offset);
                auto inst1 = new Inst_Stack_Offset(_reg + 1, decl->data_offset);
                copy_location_info(inst1, decl);
                bytecode->push_back(inst1);
				printf("\tBYTECODE_STORE %zd, %zd, %lld\n", _reg + 1, _reg, ty_decl->byte_size);
                auto inst2 = new Inst_Store(_reg + 1, _reg, ty_decl->byte_size);
                copy_location_info(inst2, decl);
                bytecode->push_back(inst2);
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
	// TODO: handle initialization of global variables
	if (!bytecode) return reg;
	switch (lit->literal_type) {
		case AST_LITERAL_SIGNED_INT: {
            // TODO: handle different number sizes
			printf("\tBYTECODE_SET_INTEGER %zd, 8, %lld\n", reg, lit->int_value);
            auto inst = new Inst_Set_Integer(reg, lit->int_value);
            copy_location_info(inst, lit);
            bytecode->push_back(inst);
			return reg;
		}
		case AST_LITERAL_UNSIGNED_INT: {
            // TODO: handle different number sizes
			printf("\tBYTECODE_SET_INTEGER %zd, 8, %llu\n", reg, lit->uint_value);
            auto inst = new Inst_Set_Integer(reg, lit->uint_value);
            copy_location_info(inst, lit);
            bytecode->push_back(inst);
			return reg;
		}
		case AST_LITERAL_DECIMAL: {
            // TODO: handle different number sizes
			printf("\tBYTECODE_SET_DECIMAL %zd, 8, 0x%llf\n", reg, lit->decimal_value);
            // TODO: build & use BYTECODE_SET_DECIMAL instruction
			return reg;
		}
		default: {
			Light_Compiler::inst->error_stop(lit, "Literal type to bytecode conversion not supported!");
			return reg;
		}
	}
}

size_t Bytecode_Generator::gen (Ast_Binary* binop, vector<Instruction*>* bytecode, size_t reg) {
	size_t rhs_reg = this->gen(binop->rhs, bytecode, reg);
	size_t lhs_reg = this->gen(binop->lhs, bytecode, rhs_reg + 1);
	switch (binop->binary_op) {
		case AST_BINARY_ADD: {
			printf("\tBYTECODE_ADD %zd, %zd\n", rhs_reg, lhs_reg);
            auto inst1 = new Inst_Add(rhs_reg, lhs_reg);
            copy_location_info(inst1, binop);
            bytecode->push_back(inst1);
			return rhs_reg;
		}
		case AST_BINARY_SUB: {
			printf("\tBYTECODE_SUB %zd, %zd\n", rhs_reg, lhs_reg);
            auto inst1 = new Inst_Sub(rhs_reg, lhs_reg);
            copy_location_info(inst1, binop);
            bytecode->push_back(inst1);
			return rhs_reg;
		}
		case AST_BINARY_MUL: {
			printf("\tBYTECODE_MUL %zd, %zd\n", rhs_reg, lhs_reg);
            auto inst1 = new Inst_Mul(rhs_reg, lhs_reg);
            copy_location_info(inst1, binop);
            bytecode->push_back(inst1);
			return rhs_reg;
		}
		case AST_BINARY_DIV: {
			printf("\tBYTECODE_DIV %zd, %zd\n", rhs_reg, lhs_reg);
            auto inst1 = new Inst_Div(rhs_reg, lhs_reg);
            copy_location_info(inst1, binop);
            bytecode->push_back(inst1);
			return rhs_reg;
		}
		default: return reg;
	}
}

size_t Bytecode_Generator::gen (Ast_Unary* unop, vector<Instruction*>* bytecode, size_t reg) {
	size_t next_reg = this->gen(unop->exp, bytecode, reg);
	switch (unop->unary_op) {
		case AST_UNARY_NEGATE: {
			printf("\tBYTECODE_NEG %zd\n", next_reg);
			return next_reg;
		}
		case AST_UNARY_NOT: {
			printf("\tBYTECODE_NOT %zd\n", next_reg);
			return next_reg;
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
            auto inst1 = new Inst_Stack_Offset(reg + 1, ident->declaration->data_offset);
            copy_location_info(inst1, ident);
            bytecode->push_back(inst1);
			printf("\tBYTECODE_DEREFERENCE %zd, %zd, %zd\n", reg, reg + 1, ident->inferred_type->byte_size);
            auto inst2 = new Inst_Dereference(reg, reg + 1, ident->inferred_type->byte_size);
            copy_location_info(inst2, ident);
            bytecode->push_back(inst2);
		} else {
			printf("\tBYTECODE_STACK_OFFSET %zd, %zd\n", reg, ident->declaration->data_offset);
		}
		return reg;
	}
}

size_t Bytecode_Generator::gen (Ast_Function* fn, vector<Instruction*>* bytecode, size_t reg) {
	if (fn->scope) this->gen(fn->scope, &fn->bytecode, 0);

	return reg;
}
