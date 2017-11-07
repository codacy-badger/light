#include "bytecode/bytecode_generator.hpp"

#include "cff.hpp"

#include <windows.h>

void Bytecode_Generator::on_statement (Ast_Statement* stm) {
    this->gen(stm);
    this->to_next(stm);
}

void Bytecode_Generator::gen (Ast_Statement* stm, vector<Instruction*>* bytecode) {
    //printf("GEN Ast_Statement\n");
    switch (stm->stm_type) {
        case AST_STATEMENT_DECLARATION: {
            this->gen(static_cast<Ast_Declaration*>(stm));
            break;
        }
        default: return;
    }
}

void Bytecode_Generator::gen (Ast_Block* block, vector<Instruction*>* bytecode) {
    //printf("GEN Ast_Block\n");
    for (auto stm : block->list) {
		this->gen(stm, bytecode);
	};
}

void Bytecode_Generator::gen (Ast_Declaration* decl, vector<Instruction*>* bytecode) {
    //printf("GEN Ast_Declaration\n");
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
				/* auto reg = */ this->gen(decl->expression);
				// BYTECODE_GLOBAL_PLUS_OFFSET
				// BYTECODE_STORE_THROUGH_REGISTER
			}
		} else {
			printf("Declaration inside function: %s\n", decl->name);
			// TODO: reserve space in the stack
			// TODO: store ptr to stack in declaration

			if (decl->expression) {
				/* auto reg = */ this->gen(decl->expression);
				// TODO: store result in ptr
			}
		}
	}
}

void Bytecode_Generator::gen (Ast_Expression* exp, vector<Instruction*>* bytecode) {
    //printf("GEN Ast_Expression\n");
    switch (exp->exp_type) {
        case AST_EXPRESSION_FUNCTION: {
            this->gen(static_cast<Ast_Function*>(exp));
            break;
        }
        default: return;
    }
}

void Bytecode_Generator::gen (Ast_Function* fn, vector<Instruction*>* bytecode) {
    printf("GEN Ast_Function: %s\n", fn->name);

	if (fn->scope) {
		this->gen(fn->scope, &fn->bytecode);
	}
}
