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
		case AST_STATEMENT_EXPRESSION: {
			this->gen(static_cast<Ast_Expression*>(stm), bytecode, reg);
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
    printf("\tBYTECODE_COPY 0, %lld\n", ret_reg);
    auto inst = new Inst_Copy(0, ret_reg);
    copy_location_info(inst, ret);
    bytecode->push_back(inst);
    printf("\tBYTECODE_RETURN\n");
    auto inst2 = new Inst_Return();
    copy_location_info(inst2, ret);
    bytecode->push_back(inst2);
}

void Bytecode_Generator::gen (Ast_Declaration* decl, vector<Instruction*>* bytecode, size_t reg) {
    if (decl->decl_flags & DECL_FLAG_CONSTANT) {
		if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
            auto func = static_cast<Ast_Function*>(decl->expression);
            if (!func->foreign_module_name) {
    			auto _tmp = this->stack_offset;
    			this->gen(func, bytecode, reg);
                this->stack_offset = _tmp;
            }
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
			}
		}
	}
}

size_t Bytecode_Generator::gen (Ast_Expression* exp, vector<Instruction*>* bytecode, size_t reg, bool address) {
    switch (exp->exp_type) {
        case AST_EXPRESSION_LITERAL: return this->gen(static_cast<Ast_Literal*>(exp), bytecode, reg);
		case AST_EXPRESSION_UNARY: return this->gen(static_cast<Ast_Unary*>(exp), bytecode, reg);
        case AST_EXPRESSION_BINARY: return this->gen(static_cast<Ast_Binary*>(exp), bytecode, reg);
        case AST_EXPRESSION_IDENT: return this->gen(static_cast<Ast_Ident*>(exp), bytecode, reg, address);
        //case AST_EXPRESSION_FUNCTION: return this->gen(static_cast<Ast_Function*>(exp), bytecode, reg);
        case AST_EXPRESSION_CALL: return this->gen(static_cast<Ast_Function_Call*>(exp), bytecode, reg);
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
	switch (binop->binary_op) {
		case AST_BINARY_ASSIGN: {
            auto size = binop->rhs->inferred_type->byte_size;
        	size_t lhs_reg = this->gen(binop->lhs, bytecode, reg, true);
        	size_t rhs_reg = this->gen(binop->rhs, bytecode, lhs_reg + 1);
			printf("\tBYTECODE_STORE %zd, %zd, %lld\n", lhs_reg, rhs_reg, size);
            auto inst2 = new Inst_Store(lhs_reg, rhs_reg, size);
            copy_location_info(inst2, binop);
            bytecode->push_back(inst2);
			return lhs_reg;
		}
		case AST_BINARY_ADD: {
        	size_t lhs_reg = this->gen(binop->lhs, bytecode, reg);
        	size_t rhs_reg = this->gen(binop->rhs, bytecode, lhs_reg + 1);
			printf("\tBYTECODE_ADD %zd, %zd\n", lhs_reg, rhs_reg);
            auto inst1 = new Inst_Add(lhs_reg, rhs_reg);
            copy_location_info(inst1, binop);
            bytecode->push_back(inst1);
			return lhs_reg;
		}
		case AST_BINARY_SUB: {
        	size_t lhs_reg = this->gen(binop->lhs, bytecode, reg);
        	size_t rhs_reg = this->gen(binop->rhs, bytecode, lhs_reg + 1);
			printf("\tBYTECODE_SUB %zd, %zd\n", lhs_reg, rhs_reg);
            auto inst1 = new Inst_Sub(lhs_reg, rhs_reg);
            copy_location_info(inst1, binop);
            bytecode->push_back(inst1);
			return lhs_reg;
		}
		case AST_BINARY_MUL: {
        	size_t lhs_reg = this->gen(binop->lhs, bytecode, reg);
        	size_t rhs_reg = this->gen(binop->rhs, bytecode, lhs_reg + 1);
			printf("\tBYTECODE_MUL %zd, %zd\n", lhs_reg, rhs_reg);
            auto inst1 = new Inst_Mul(lhs_reg, rhs_reg);
            copy_location_info(inst1, binop);
            bytecode->push_back(inst1);
			return lhs_reg;
		}
		case AST_BINARY_DIV: {
        	size_t lhs_reg = this->gen(binop->lhs, bytecode, reg);
        	size_t rhs_reg = this->gen(binop->rhs, bytecode, lhs_reg + 1);
			printf("\tBYTECODE_DIV %zd, %zd\n", lhs_reg, rhs_reg);
            auto inst1 = new Inst_Div(lhs_reg, rhs_reg);
            copy_location_info(inst1, binop);
            bytecode->push_back(inst1);
			return lhs_reg;
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

size_t Bytecode_Generator::gen (Ast_Ident* ident, vector<Instruction*>* bytecode, size_t reg, bool address) {
	if (ident->declaration->is_global()) {
		if (ident->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
			printf("\tBYTECODE_LOAD_GLOBAL %zd, %zd, %zd (%s)\n", reg, ident->declaration->data_offset, ident->inferred_type->byte_size, ident->name);
		} else {
			printf("\tBYTECODE_LOAD_GLOBAL_POINTER %zd, %zd (%s)\n", reg, ident->declaration->data_offset, ident->name);
		}
		return reg;
	} else {
		if (ident->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
			printf("\tBYTECODE_STACK_OFFSET %zd, %zd\n", reg, ident->declaration->data_offset);
            auto inst1 = new Inst_Stack_Offset(reg, ident->declaration->data_offset);
            copy_location_info(inst1, ident);
            bytecode->push_back(inst1);
            if (!address) {
    			printf("\tBYTECODE_LOAD %zd, %zd, %zd\n", reg, reg, ident->inferred_type->byte_size);
                auto inst2 = new Inst_Load(reg, reg, ident->inferred_type->byte_size);
                copy_location_info(inst2, ident);
                bytecode->push_back(inst2);
            }
		} else {
			printf("\tBYTECODE_STACK_OFFSET %zd, %zd\n", reg, ident->declaration->data_offset);
		}
		return reg;
	}
}

size_t Bytecode_Generator::gen (Ast_Function* fn, vector<Instruction*>* bytecode, size_t reg) {
    auto free_reg = fn->type->parameter_decls.size();
    for (int i = 0; i < fn->type->parameter_decls.size(); i++) {
        auto decl = fn->type->parameter_decls[i];

        auto decl_type = static_cast<Ast_Type_Definition*>(decl->type);
        auto size = decl_type->byte_size;
        decl->data_offset = this->stack_offset;
        this->stack_offset += size;

        printf("\tBYTECODE_STACK_ALLOCATE %zd\n", size);
        auto inst = new Inst_Stack_Allocate(size);
        copy_location_info(inst, decl);
        fn->bytecode.push_back(inst);
        printf("\tBYTECODE_STACK_OFFSET %zd, %zd\n", free_reg, decl->data_offset);
        auto inst1 = new Inst_Stack_Offset(free_reg, decl->data_offset);
        copy_location_info(inst1, decl);
        fn->bytecode.push_back(inst1);
        printf("\tBYTECODE_STORE %zd, %d, %lld\n", free_reg, i, size);
        auto inst2 = new Inst_Store(free_reg, i, size);
        copy_location_info(inst2, decl);
        fn->bytecode.push_back(inst2);
    }
	if (fn->scope) this->gen(fn->scope, &fn->bytecode, 0);
    // TODO: if there's no return instructions (void) add one at the end
	return reg;
}

size_t Bytecode_Generator::gen (Ast_Function_Call* call, vector<Instruction*>* bytecode, size_t reg) {
	auto func = static_cast<Ast_Function*>(call->fn);
	printf("\tBYTECODE_CALL_SETUP %d, %d\n", BYTECODE_CC_CDECL, !!func->foreign_module_name);

    auto inst1 = new Inst_Call_Setup(BYTECODE_CC_CDECL, !!func->foreign_module_name);
    copy_location_info(inst1, call);
    bytecode->push_back(inst1);

	auto param_reg = reg;
	auto bytecode_type = BYTECODE_TYPE_VOID;
	for (int i = 0; i < call->parameters.size(); i++) {
		auto exp = call->parameters[i];
		bytecode_type = bytecode_get_type(exp->inferred_type);
		param_reg = this->gen(exp, bytecode, param_reg);
		printf("\tBYTECODE_CALL_PARAM %d, %zd, %d\n", i, param_reg, bytecode_type);

        auto inst2 = new Inst_Call_Param(i, param_reg, bytecode_type);
        copy_location_info(inst2, call);
        bytecode->push_back(inst2);

	}
	if (func->foreign_module_name) {
		bytecode_type = bytecode_get_type(func->type->return_type);
		size_t module_index, function_index;
		Light_Compiler::inst->interp->foreign_functions->store(func->foreign_module_name, func->name, &module_index, &function_index);
		printf("\tBYTECODE_CALL_FOREIGN %zd, %zd, %zd, %d (%s @ %s)\n", reg, module_index, function_index, bytecode_type,
			Light_Compiler::inst->interp->foreign_functions->function_names[function_index].c_str(),
			Light_Compiler::inst->interp->foreign_functions->module_names[module_index].c_str());

        auto inst2 = new Inst_Call_Foreign(reg, module_index, function_index, bytecode_type);
        copy_location_info(inst2, call);
        bytecode->push_back(inst2);
	} else {
		printf("\tBYTECODE_CALL %zd, %p (%s)\n", reg, func, func->name);

        auto inst2 = new Inst_Call(reg, reinterpret_cast<size_t>(func));
        copy_location_info(inst2, call);
        bytecode->push_back(inst2);
	}
	return reg;
}
