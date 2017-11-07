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
		if (decl->type->exp_type == AST_EXPRESSION_TYPE_DEFINITION) {
			auto ty_defn = static_cast<Ast_Type_Definition*>(decl->type);
			if (ty_defn->typedef_type == AST_TYPEDEF_FUNCTION) {
				this->gen(static_cast<Ast_Function*>(decl->expression));
			}
		} else {
			// auto offset = Light_Compiler::instance->interp->const->store(decl->expression);
			// decl->data_offset = offset;
		}
    } else {
		if (decl->scope->is_global()) {
			printf("Declaration is global: %s\n", decl->name);
			// TODO: reserve space in the global storage
			// auto offset = Light_Compiler::instance->interp->global->reserve(decl->type->size);
			// decl->data_offset = offset;

			if (decl->expression) {
				auto _reg = this->gen(decl->expression);
				// BYTECODE_GLOBAL_PLUS_OFFSET
				// BYTECODE_STORE_THROUGH_REGISTER
			}
		} else {
			printf("Declaration inside function: %s\n", decl->name);
			// TODO: reserve space in the stack
			// TODO: store ptr to stack in declaration

			if (decl->expression) {
				auto _reg = this->gen(decl->expression);
				// TODO: store result in ptr
			}
		}
	}
}

size_t Bytecode_Generator::gen (Ast_Expression* exp, vector<Instruction*>* bytecode, size_t reg) {
    switch (exp->exp_type) {
        case AST_EXPRESSION_LITERAL: return this->gen(static_cast<Ast_Literal*>(exp), bytecode, reg);
		case AST_EXPRESSION_UNARY: return this->gen(static_cast<Ast_Unary*>(exp), bytecode, reg);
        case AST_EXPRESSION_BINARY: return this->gen(static_cast<Ast_Binary*>(exp), bytecode, reg);
        case AST_EXPRESSION_FUNCTION: return this->gen(static_cast<Ast_Function*>(exp), bytecode, reg);
        default: return reg;
    }
}

size_t Bytecode_Generator::gen (Ast_Literal* lit, vector<Instruction*>* bytecode, size_t reg) {
	switch (lit->literal_type) {
		case AST_LITERAL_I64: {
			printf("BYTECODE_COPY_CONST %lld, 8, 0x%llX\n", reg, lit->i64_value);
			return reg;
		}
		case AST_LITERAL_U64: {
			printf("BYTECODE_COPY_CONST %lld, 8, 0x%llX\n", reg, lit->u64_value);
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
		case AST_UNARY_NEGATE_MATH: {
			printf("BYTECODE_NEGATE_MATH %lld\n", next_reg);
			return reg;
		}
		case AST_UNARY_NEGATE_LOGIC: {
			printf("BYTECODE_NEGATE_LOGIC %lld\n", next_reg);
			return reg;
		}
		default: return reg;
	}
}

size_t Bytecode_Generator::gen (Ast_Binary* binop, vector<Instruction*>* bytecode, size_t reg) {
	return reg;
}

size_t Bytecode_Generator::gen (Ast_Function* fn, vector<Instruction*>* bytecode, size_t reg) {
	if (fn->scope) {
		this->gen(fn->scope, &fn->bytecode);
	}

	return reg;
}
