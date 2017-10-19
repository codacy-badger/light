#pragma once

#include "back/llvm/llvm_pipe.hpp"

#include "compiler.hpp"
#include "timer.hpp"

LLVMPipe::LLVMPipe (const char* output) : builder(context) {
	module = new Module(output, context);
}

void LLVMPipe::onStatement (Ast_Statement* stm) {
    this->codegen(stm);
}

void LLVMPipe::onFinish () {
	module->print(outs(), NULL);
	/*auto start = clock();
	LLVMObjWritter::writeObj(module);
	Timer::print("  Write ", start);*/
}

Value* LLVMPipe::codegen (Ast_Statement* stm) {
    switch (stm->stm_type) {
        case AST_STATEMENT_BLOCK: {
            return codegen(static_cast<Ast_Block*>(stm));
        }
        case AST_STATEMENT_DECLARATION: {
            return codegen(static_cast<Ast_Declaration*>(stm));
        }
        case AST_STATEMENT_EXPRESSION: {
            return codegen(static_cast<Ast_Expression*>(stm));
        }
        case AST_STATEMENT_RETURN: {
            return codegen(static_cast<Ast_Return*>(stm));
        }
        default: return NULL;
    }
}

Value* LLVMPipe::codegen (Ast_Block* stms) {
	for(auto const& stm: stms->list) {
		this->codegen(stm);
		if (stm->stm_type == AST_STATEMENT_RETURN)
            return NULL;
	}
	return NULL;
}

Value* LLVMPipe::codegen (Ast_Declaration* decl, bool alloca) {
    return NULL;
}

Value* LLVMPipe::codegen (Ast_Return* ret) {
    return NULL;
}

Value* LLVMPipe::codegen (Ast_Expression* exp) {
    switch (exp->exp_type) {
        case AST_EXPRESSION_FUNCTION: {
            return codegen(static_cast<Ast_Function*>(exp));
        }
        case AST_EXPRESSION_TYPE_DEFINITION: {
            codegen(static_cast<Ast_Type_Definition*>(exp));
            return NULL;
        }
        case AST_EXPRESSION_BINARY: {
            return codegen(static_cast<Ast_Binary*>(exp));
        }
        case AST_EXPRESSION_UNARY: {
            return codegen(static_cast<Ast_Unary*>(exp));
        }
        case AST_EXPRESSION_CALL: {
            return codegen(static_cast<Ast_Function_Call*>(exp));
        }
        case AST_EXPRESSION_IDENT: {
            return codegen(static_cast<Ast_Ident*>(exp));
        }
        case AST_EXPRESSION_LITERAL: {
            return codegen(static_cast<Ast_Literal*>(exp));
        }
        default: return NULL;
    }
}

Value* LLVMPipe::codegen (Ast_Function* fn) {
    return NULL;
}

Type* LLVMPipe::codegen (Ast_Type_Definition* tyDef) {
    return NULL;
}

Type* LLVMPipe::codegen (Ast_Type_Instance* ty) {
	return NULL;
}

Value* LLVMPipe::codegen (Ast_Binary* binop) {
	switch (binop->binary_op) {
		case AST_BINARY_ADD:
			return LLVMCodegenPrimitive::add(&builder,
				this->codegen(binop->lhs), this->codegen(binop->rhs));
		case AST_BINARY_SUB:
			return LLVMCodegenPrimitive::sub(&builder,
				this->codegen(binop->lhs), this->codegen(binop->rhs));
		case AST_BINARY_MUL:
			return LLVMCodegenPrimitive::mul(&builder,
				this->codegen(binop->lhs), this->codegen(binop->rhs));
		case AST_BINARY_DIV:
			return LLVMCodegenPrimitive::div(&builder,
				this->codegen(binop->lhs), this->codegen(binop->rhs));
		case AST_BINARY_ASSIGN: {
			printf("Assign!\n");
		}
		default: break;
	}
	return NULL;
}

Value* LLVMPipe::codegen (Ast_Unary* unop) {
	Value* val = this->codegen(unop->exp);
	switch (unop->unary_op) {
		case AST_UNARY_NEGATE_NUMBER:
			return LLVMCodegenPrimitive::neg(&builder, val);
		default: break;
	}
	return NULL;
}

Value* LLVMPipe::codegen (Ast_Ident* ident) {
	return NULL;
}

Value* LLVMPipe::codegen (Ast_Literal* con) {
	return NULL;
}

Value* LLVMPipe::codegen (Ast_Function_Call* call) {
	return NULL;
}
