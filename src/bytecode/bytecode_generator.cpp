#include "bytecode/bytecode_generator.hpp"

#include "compiler.hpp"

Instruction* copy_location_info (Instruction* intruction, Ast* node) {
    intruction->filename = node->filename;
    intruction->line = node->line;
	return intruction;
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
	if (ret->exp) {
	    auto ret_reg = this->gen(ret->exp, bytecode, reg);
	    if (ret_reg != 0) {
	        auto inst = new Inst_Copy(0, ret_reg);
	        bytecode->push_back(copy_location_info(inst, ret));
	    }
	}
    auto inst2 = new Inst_Return();
    bytecode->push_back(copy_location_info(inst2, ret));
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
            auto inst = new Inst_Stack_Allocate(ty_decl->byte_size);
            ;
            bytecode->push_back(copy_location_info(inst, decl));

			decl->data_offset = this->stack_offset;
			this->stack_offset += ty_decl->byte_size;
			if (decl->expression) {
				auto _reg = this->gen(decl->expression, bytecode, reg);
                auto inst1 = new Inst_Stack_Offset(_reg + 1, decl->data_offset);
                bytecode->push_back(copy_location_info(inst1, decl));
                auto inst2 = new Inst_Store(_reg + 1, _reg, ty_decl->byte_size);
                bytecode->push_back(copy_location_info(inst2, decl));
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
	auto instance = Light_Compiler::inst;

	switch (lit->literal_type) {
		case AST_LITERAL_SIGNED_INT: {
			auto bytecode_type = bytecode_get_type(lit->inferred_type);
			auto inst = new Inst_Set(reg, bytecode_type, &lit->int_value);
            bytecode->push_back(copy_location_info(inst, lit));
			return reg;
		}
		case AST_LITERAL_UNSIGNED_INT: {
			auto bytecode_type = bytecode_get_type(lit->inferred_type);
			auto inst = new Inst_Set(reg, bytecode_type, &lit->uint_value);
            bytecode->push_back(copy_location_info(inst, lit));
			return reg;
		}
		case AST_LITERAL_DECIMAL: {
			auto bytecode_type = bytecode_get_type(lit->inferred_type);
			auto inst = new Inst_Set(reg, bytecode_type, &lit->decimal_value);
            bytecode->push_back(copy_location_info(inst, lit));
			return reg;
		}
		case AST_LITERAL_STRING: {
			lit->data_offset = Light_Compiler::inst->interp->constants->add(lit->string_value);
            auto inst = new Inst_Constant_Offset(reg, lit->data_offset);
            bytecode->push_back(copy_location_info(inst, lit));
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
        	size_t rhs_reg = this->gen(binop->rhs, bytecode, reg);
        	size_t lhs_reg = this->gen(binop->lhs, bytecode, rhs_reg + 1, true);
            auto inst2 = new Inst_Store(lhs_reg, rhs_reg, size);
            bytecode->push_back(copy_location_info(inst2, binop));
			return lhs_reg;
		}
		case AST_BINARY_ADD: {
        	size_t lhs_reg = this->gen(binop->lhs, bytecode, reg);
        	size_t rhs_reg = this->gen(binop->rhs, bytecode, lhs_reg + 1);
            auto inst1 = new Inst_Add(lhs_reg, rhs_reg);
            bytecode->push_back(copy_location_info(inst1, binop));
			return lhs_reg;
		}
		case AST_BINARY_SUB: {
        	size_t lhs_reg = this->gen(binop->lhs, bytecode, reg);
        	size_t rhs_reg = this->gen(binop->rhs, bytecode, lhs_reg + 1);
            auto inst1 = new Inst_Sub(lhs_reg, rhs_reg);
            bytecode->push_back(copy_location_info(inst1, binop));
			return lhs_reg;
		}
		case AST_BINARY_MUL: {
        	size_t lhs_reg = this->gen(binop->lhs, bytecode, reg);
        	size_t rhs_reg = this->gen(binop->rhs, bytecode, lhs_reg + 1);
            auto inst1 = new Inst_Mul(lhs_reg, rhs_reg);
            bytecode->push_back(copy_location_info(inst1, binop));
			return lhs_reg;
		}
		case AST_BINARY_DIV: {
        	size_t lhs_reg = this->gen(binop->lhs, bytecode, reg);
        	size_t rhs_reg = this->gen(binop->rhs, bytecode, lhs_reg + 1);
            auto inst1 = new Inst_Div(lhs_reg, rhs_reg);
            bytecode->push_back(copy_location_info(inst1, binop));
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
            auto inst1 = new Inst_Stack_Offset(reg, ident->declaration->data_offset);
            bytecode->push_back(copy_location_info(inst1, ident));
            if (!address) {
                auto inst2 = new Inst_Load(reg, reg, ident->inferred_type->byte_size);
                bytecode->push_back(copy_location_info(inst2, ident));
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

        auto inst = new Inst_Stack_Allocate(size);
        fn->bytecode.push_back(copy_location_info(inst, decl));
        auto inst1 = new Inst_Stack_Offset(free_reg, decl->data_offset);
        fn->bytecode.push_back(copy_location_info(inst1, decl));
        auto inst2 = new Inst_Store(free_reg, i + 1, size);
        fn->bytecode.push_back(copy_location_info(inst2, decl));
    }
	if (fn->scope) this->gen(fn->scope, &fn->bytecode, 0);
    // TODO: if there's no return instructions (void) add one at the end
	return reg;
}

size_t Bytecode_Generator::gen (Ast_Function_Call* call, vector<Instruction*>* bytecode, size_t reg) {
	auto func = static_cast<Ast_Function*>(call->fn);

	std::vector<int64_t> stack_offsets;
	for (int i = 0; i < call->parameters.size(); i++) {
		auto exp = call->parameters[i];
		if (exp->exp_type == AST_EXPRESSION_CALL) {
			auto call = static_cast<Ast_Function_Call*>(exp);
			this->gen(call, bytecode, reg);

			auto bytecode_type = bytecode_get_type(call->inferred_type);
			auto size = bytecode_get_size(bytecode_type);
			stack_offsets.push_back(this->stack_offset);

			auto inst = new Inst_Stack_Allocate(size);
	        bytecode->push_back(copy_location_info(inst, call));
	        auto inst1 = new Inst_Stack_Offset(reg + 1, this->stack_offset);
	        bytecode->push_back(copy_location_info(inst1, call));
	        auto inst2 = new Inst_Store(reg + 1, reg, size);
	        bytecode->push_back(copy_location_info(inst2, call));
			this->stack_offset += size;
		} else stack_offsets.push_back(-1);
	}

    auto inst1 = new Inst_Call_Setup(BYTECODE_CC_CDECL, func->foreign_module_name);
    copy_location_info(inst1, call);
    bytecode->push_back(inst1);

	auto bytecode_type = BYTECODE_TYPE_VOID;
	for (int i = 0; i < call->parameters.size(); i++) {
		auto exp = call->parameters[i];
		bytecode_type = bytecode_get_type(exp->inferred_type);

		if (stack_offsets[i] > -1) {
			auto size = bytecode_get_size(bytecode_type);

			auto inst1 = new Inst_Stack_Offset(i + 1, stack_offsets[i]);
	        bytecode->push_back(copy_location_info(inst1, exp));
	        auto inst2 = new Inst_Load(i + 1, i + 1, size);
	        bytecode->push_back(copy_location_info(inst2, exp));

	        auto inst3 = new Inst_Call_Param(i, bytecode_type);
	        bytecode->push_back(copy_location_info(inst3, call));
		} else {
			auto _reg = this->gen(exp, bytecode, i + 1);
			assert(_reg == i + 1);

	        auto inst2 = new Inst_Call_Param(i, bytecode_type);
	        bytecode->push_back(copy_location_info(inst2, call));
		}
	}

	if (func->foreign_module_name) {
		Light_Compiler::inst->interp->foreign_functions->store(func->foreign_module_name, func->name);
	}
    auto inst2 = new Inst_Call(reinterpret_cast<size_t>(func));
    bytecode->push_back(copy_location_info(inst2, call));
	return reg;
}
