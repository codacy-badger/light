#include "bytecode/bytecode_generator.hpp"

#include "compiler.hpp"

Instruction* copy_location_info (Instruction* intruction, Ast* node) {
    intruction->filename = node->filename;
    intruction->line = node->line;
	return intruction;
}

void Bytecode_Generator::on_statement (Ast_Statement* stm) {
    this->gen(stm, NULL);
    this->to_next(stm);
}

void Bytecode_Generator::gen (Ast_Statement* stm, vector<Instruction*>* bytecode) {
    switch (stm->stm_type) {
        case AST_STATEMENT_DECLARATION: {
            this->gen(static_cast<Ast_Declaration*>(stm), bytecode);
            break;
        }
		case AST_STATEMENT_RETURN: {
			this->gen(static_cast<Ast_Return*>(stm), bytecode);
			break;
		}
		case AST_STATEMENT_EXPRESSION: {
			this->gen(static_cast<Ast_Expression*>(stm), bytecode);
			break;
		}
        default: return;
    }
}

void Bytecode_Generator::gen (Ast_Block* block, vector<Instruction*>* bytecode) {
    for (auto stm : block->list) {
		this->gen(stm, bytecode);
	};
}

void Bytecode_Generator::gen (Ast_Return* ret, vector<Instruction*>* bytecode) {
	if (ret->exp) {
	    this->gen(ret->exp, bytecode);
	    if (this->current_register != 1) {
	        auto inst = new Inst_Copy(0, this->current_register - 1);
	        bytecode->push_back(copy_location_info(inst, ret));
	    }
	}
    auto inst2 = new Inst_Return();
    bytecode->push_back(copy_location_info(inst2, ret));
}

void Bytecode_Generator::gen (Ast_Declaration* decl, vector<Instruction*>* bytecode) {
    if (decl->decl_flags & DECL_FLAG_CONSTANT) {
		if (decl->expression->exp_type == AST_EXPRESSION_FUNCTION) {
            auto func = static_cast<Ast_Function*>(decl->expression);
            if (!func->foreign_module_name) {
    			auto _tmp = this->stack_offset;
    			this->gen(func, bytecode);
                this->stack_offset = _tmp;
            }
		}
    } else {
		auto ty_decl = static_cast<Ast_Type_Definition*>(decl->type);
		if (decl->scope->is_global()) {
            auto ty_defn = static_cast<Ast_Type_Definition*>(decl->type);
			decl->stack_offset = this->global_offset;
            this->global_offset += ty_defn->byte_size;

			if (decl->expression) {
				this->gen(decl->expression, bytecode);
				printf("\tBYTECODE_GLOBAL_OFFSET %d, %lld\n", 1, decl->stack_offset);
				printf("\tBYTECODE_STORE %d, %d, %lld\n", 1, 0, ty_decl->byte_size);
			}
		} else {
            auto inst = new Inst_Stack_Allocate(ty_decl->byte_size);
            bytecode->push_back(copy_location_info(inst, decl));

			decl->stack_offset = this->stack_offset;
			this->stack_offset += ty_decl->byte_size;
			if (decl->expression) {
				this->gen(decl->expression, bytecode);
                auto inst1 = new Inst_Stack_Offset(1, decl->stack_offset);
                bytecode->push_back(copy_location_info(inst1, decl));
                auto inst2 = new Inst_Store(1, 0, ty_decl->byte_size);
                bytecode->push_back(copy_location_info(inst2, decl));
			}
		}
	}
}

void Bytecode_Generator::gen (Ast_Expression* exp, vector<Instruction*>* bytecode, bool address) {
    switch (exp->exp_type) {
        case AST_EXPRESSION_LITERAL: return this->gen(static_cast<Ast_Literal*>(exp), bytecode);
		case AST_EXPRESSION_UNARY: return this->gen(static_cast<Ast_Unary*>(exp), bytecode);
        case AST_EXPRESSION_BINARY: return this->gen(static_cast<Ast_Binary*>(exp), bytecode);
        case AST_EXPRESSION_IDENT: return this->gen(static_cast<Ast_Ident*>(exp), bytecode, address);
        case AST_EXPRESSION_CALL: return this->gen(static_cast<Ast_Function_Call*>(exp), bytecode);
        default: return;
    }
}

void Bytecode_Generator::gen (Ast_Literal* lit, vector<Instruction*>* bytecode) {
	switch (lit->literal_type) {
		case AST_LITERAL_SIGNED_INT:
		case AST_LITERAL_UNSIGNED_INT:
		case AST_LITERAL_DECIMAL: {
			auto bytecode_type = bytecode_get_type(lit->inferred_type);
			auto inst = new Inst_Set(this->current_register++, bytecode_type, &lit->int_value);
            bytecode->push_back(copy_location_info(inst, lit));
			break;
		}
		case AST_LITERAL_STRING: {
			lit->data_offset = Light_Compiler::inst->interp->constants->add(lit->string_value);
            auto inst = new Inst_Constant_Offset(this->current_register++, lit->data_offset);
            bytecode->push_back(copy_location_info(inst, lit));
			break;
		}
		default: {
			Light_Compiler::inst->error_stop(lit, "Literal type to bytecode conversion not supported!");
		}
	}
}

uint8_t get_bytecode_from_binop (Ast_Binary_Type binop) {
	switch (binop) {
		case AST_BINARY_ADD: return BYTECODE_ADD;
		case AST_BINARY_SUB: return BYTECODE_SUB;
		case AST_BINARY_MUL: return BYTECODE_MUL;
		case AST_BINARY_DIV: return BYTECODE_DIV;
	}
	return BYTECODE_NOOP;
}

void Bytecode_Generator::gen (Ast_Binary* binop, vector<Instruction*>* bytecode) {
	auto reg = this->current_register++;
	switch (binop->binary_op) {
		case AST_BINARY_ASSIGN: {
            auto size = binop->rhs->inferred_type->byte_size;
        	this->gen(binop->lhs, bytecode, true);
        	this->gen(binop->rhs, bytecode);
            auto inst2 = new Inst_Store(reg - 1, reg, size);
            bytecode->push_back(copy_location_info(inst2, binop));
			break;
		}
		case AST_BINARY_ADD:
		case AST_BINARY_SUB:
		case AST_BINARY_MUL:
		case AST_BINARY_DIV: {
			this->gen(binop->lhs, bytecode);
			this->gen(binop->rhs, bytecode);
			auto binop_type = get_bytecode_from_binop(binop->binary_op);
            auto inst1 = new Inst_Binary(binop_type, reg - 1, reg);
            bytecode->push_back(copy_location_info(inst1, binop));
			break;
		}
		default: return;
	}
}

void Bytecode_Generator::gen (Ast_Unary* unop, vector<Instruction*>* bytecode) {
	this->gen(unop->exp, bytecode);
	switch (unop->unary_op) {
		case AST_UNARY_NEGATE: {
			printf("\tBYTECODE_NEG %zd\n", this->current_register);
			break;
		}
		case AST_UNARY_NOT: {
			printf("\tBYTECODE_NOT %zd\n", this->current_register);
			break;
		}
		default: return;
	}
}

void Bytecode_Generator::gen (Ast_Ident* ident, vector<Instruction*>* bytecode, bool address) {
	auto reg = this->current_register++;
	if (ident->declaration->is_global()) {
		if (ident->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
			printf("\tBYTECODE_LOAD_GLOBAL %zd, %zd, %zd (%s)\n", reg, ident->declaration->stack_offset, ident->inferred_type->byte_size, ident->name);
		} else {
			printf("\tBYTECODE_LOAD_GLOBAL_POINTER %zd, %zd (%s)\n", reg, ident->declaration->stack_offset, ident->name);
		}
	} else {
		if (ident->inferred_type->byte_size <= INTERP_REGISTER_SIZE) {
            auto inst1 = new Inst_Stack_Offset(reg, ident->declaration->stack_offset);
            bytecode->push_back(copy_location_info(inst1, ident));
            if (!address) {
                auto inst2 = new Inst_Load(reg, reg, ident->inferred_type->byte_size);
                bytecode->push_back(copy_location_info(inst2, ident));
            }
		} else {
			printf("\tBYTECODE_STACK_OFFSET %zd, %zd\n", reg, ident->declaration->stack_offset);
		}
	}
}

void Bytecode_Generator::gen (Ast_Function* fn, vector<Instruction*>* bytecode) {
    auto free_reg = fn->type->parameter_decls.size();
    for (int i = 0; i < fn->type->parameter_decls.size(); i++) {
        auto decl = fn->type->parameter_decls[i];

        auto decl_type = static_cast<Ast_Type_Definition*>(decl->type);
        auto size = decl_type->byte_size;
        decl->stack_offset = this->stack_offset;
        this->stack_offset += size;

        auto inst = new Inst_Stack_Allocate(size);
        fn->bytecode.push_back(copy_location_info(inst, decl));
        auto inst1 = new Inst_Stack_Offset(free_reg, decl->stack_offset);
        fn->bytecode.push_back(copy_location_info(inst1, decl));
        auto inst2 = new Inst_Store(free_reg, i + 1, size);
        fn->bytecode.push_back(copy_location_info(inst2, decl));
    }
	if (fn->scope) {
		this->current_register = 0;
		this->gen(fn->scope, &fn->bytecode);
	}
}

void Bytecode_Generator::gen (Ast_Function_Call* call, vector<Instruction*>* bytecode) {
	auto _tmp = this->current_register;
	this->current_register = 0;
	for (int i = 0; i < call->parameters.size(); i++) {
		this->gen(call->parameters[i], bytecode);
	}
	this->current_register = _tmp + 1;

	auto inst1 = new Inst_Call_Setup(BYTECODE_CC_CDECL);
	copy_location_info(inst1, call);
	bytecode->push_back(inst1);

	auto bytecode_type = BYTECODE_TYPE_VOID;
	for (int i = 0; i < call->parameters.size(); i++) {
		auto exp = call->parameters[i];
		bytecode_type = bytecode_get_type(exp->inferred_type);
        auto inst2 = new Inst_Call_Param(i, bytecode_type);
        bytecode->push_back(copy_location_info(inst2, call));
	}

	auto func = static_cast<Ast_Function*>(call->fn);
	if (func->foreign_module_name) {
		Light_Compiler::inst->interp->foreign_functions->store(func->foreign_module_name, func->name);
	}
    auto inst2 = new Inst_Call(reinterpret_cast<size_t>(func));
    bytecode->push_back(copy_location_info(inst2, call));
}
