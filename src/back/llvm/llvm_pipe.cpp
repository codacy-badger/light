#pragma once

#include "back/llvm/llvm_pipe.hpp"

#include "timer.hpp"

LLVMPipe::LLVMPipe (std::string output) : builder(context) {
	module = new Module(output, context);
	scope = new LLVMScope(&builder);
	scope->addType(Ast_Primitive_Type::_void, Type::getVoidTy(context));
	scope->addType(Ast_Primitive_Type::_i1,   Type::getInt1Ty(context));
	scope->addType(Ast_Primitive_Type::_i8,   Type::getInt8Ty(context));
	scope->addType(Ast_Primitive_Type::_i16,  Type::getInt16Ty(context));
	scope->addType(Ast_Primitive_Type::_i32,  Type::getInt32Ty(context));
	scope->addType(Ast_Primitive_Type::_i64,  Type::getInt64Ty(context));
	scope->addType(Ast_Primitive_Type::_i128, Type::getInt128Ty(context));
}

void LLVMPipe::onFunction (Ast_Function* fn) {
	vector<Type*> argTypes;
	for(auto const& param: fn->type->parameters)
		argTypes.push_back(this->codegen(param->type));
	Type* retType = this->codegen(fn->type->retType);
	auto fnType = FunctionType::get(retType, argTypes, false);
	auto constValue = module->getOrInsertFunction(fn->name, fnType);
	auto function = static_cast<Function*>(constValue);

	int index = 0;
	for (auto& arg: function->args())
		arg.setName(fn->type->parameters[index++]->name);
	this->scope->addFunction(fn, function);

	if (fn->stm != NULL) {
		BasicBlock* prevBlock = builder.GetInsertBlock();
		this->functionHasReturned = false;
		this->currentFunction = function;

		BasicBlock* entryBlock = BasicBlock::Create(context, "entry", function);
		BasicBlock* retBlock = BasicBlock::Create(context, "return", function);
		if (fn->type->retType != Ast_Primitive_Type::_void) {
			builder.SetInsertPoint(entryBlock);
			auto type = this->codegen(fn->type->retType);
			this->currentReturnVal = builder.CreateAlloca(type, NULL, "retVal");
			builder.SetInsertPoint(retBlock);
			auto retVal = builder.CreateLoad(this->currentReturnVal);
			builder.CreateRet(retVal);
		} else {
			this->currentReturnVal = NULL;
			builder.SetInsertPoint(retBlock);
			builder.CreateRetVoid();
		}

		builder.SetInsertPoint(entryBlock);
		this->scope = scope->push();
		this->scope->addParameters(fn);
		this->codegen(fn->stm);
		this->scope = scope->pop();

		if (!builder.GetInsertBlock()->back().isTerminator()) {
			builder.CreateBr(retBlock);
		}
		builder.SetInsertPoint(prevBlock);
	}

	verifyFunction(*function);
}

void LLVMPipe::onType (Ast_Type_Definition* ty) {
	this->codegen(ty);
}

void LLVMPipe::onFinish () {
	//module->print(outs(), NULL);
	auto start = clock();
	LLVMObjWritter::writeObj(module);
	Timer::print("  Write ", start);
}

Value* LLVMPipe::codegen (Ast_Statement* stm) {
	switch (stm->stm_type) {
		case AST_STATEMENT_BLOCK: {
			codegen(static_cast<Ast_Block*>(stm));
			break;
		}
		case AST_STATEMENT_DECLARATION: {
			codegen(static_cast<Ast_Declaration*>(stm), true);
			break;
		}
		case AST_STATEMENT_RETURN: {
			codegen(static_cast<Ast_Return*>(stm));
			break;
		}
		case AST_STATEMENT_EXPRESSION: {
			codegen(static_cast<Ast_Expression*>(stm));
			break;
		}
		default: break;
	}
	return NULL;
}

Value* LLVMPipe::codegen (Ast_Block* stms) {
	for(auto const& stm: stms->list) {
		this->codegen(stm);
		if (stm->stm_type == AST_STATEMENT_RETURN) return NULL;
	}
	return NULL;
}

Value* LLVMPipe::codegen (Ast_Declaration* decl, bool alloca) {
	if (alloca) {
		auto type = this->codegen(decl->type);
		auto alloca = builder.CreateAlloca(type, NULL, decl->name);
		if (decl->expression != NULL) {
			Value* val = this->codegen(decl->expression);
			builder.CreateStore(val, alloca);
		}
		this->scope->addVariable(decl, alloca);
		return alloca;
	} else return this->scope->getVariable(decl);
}

Value* LLVMPipe::codegen (Ast_Return* ret) {
	this->functionHasReturned = false;
	if (ret->exp != NULL && this->currentReturnVal != NULL) {
		Value* retValue = this->codegen(ret->exp);
		builder.CreateStore(retValue, this->currentReturnVal);
	}
	return builder.CreateBr(&this->currentFunction->back());
}

Value* LLVMPipe::codegen (Ast_Expression* exp) {
	switch (exp->exp_type) {
		case AST_EXPRESSION_TYPE_DEFINITION: {
			codegen(static_cast<Ast_Type_Definition*>(exp));
			return NULL;
		}
		case AST_EXPRESSION_FUNCTION: {
			return codegen(static_cast<Ast_Function*>(exp));
		}
		case AST_EXPRESSION_BINARY: {
			return codegen(static_cast<AST_Binary*>(exp));
		}
		case AST_EXPRESSION_UNARY: {
			return codegen(static_cast<AST_Unary*>(exp));
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

Value* LLVMPipe::codegen (AST_Binary* binop) {
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
		default: break;
	}
	return NULL;
}

Value* LLVMPipe::codegen (AST_Unary* unop) {
	Value* val = this->codegen(unop->exp);
	switch (unop->unary_op) {
		case AST_UNARY_NEGATE_NUMBER:
			return LLVMCodegenPrimitive::neg(&builder, val);
		default: break;
	}
	return NULL;
}

Value* LLVMPipe::codegen (Ast_Literal* con) {
	switch (con->literal_type) {
		case AST_LITERAL_INTEGER:
			return ConstantInt::get(context, APInt(32, con->integer_value));
		case AST_LITERAL_STRING:
			return builder.CreateGlobalStringPtr(con->string_value);
		default: break;
	}
	return NULL;
}

Value* LLVMPipe::codegen (Ast_Function_Call* call) {
	/*Function* function = this->scope->getFunction(call->fn);

	vector<Value*> parameters;
	for(auto const& param: call->parameters)
		parameters.push_back(this->codegen(param));
	return builder.CreateCall(function, parameters, "fnCall");*/
	return NULL;
}

Type* LLVMPipe::codegen (Ast_Type_Definition* ty) {
	switch (ty->type_def_type) {
		case AST_TYPE_DEF_STRUCT:
			return codegen(static_cast<Ast_Struct_Type*>(ty));
		case AST_TYPE_DEF_POINTER:
			return codegen(static_cast<Ast_Pointer_Type*>(ty));
		default: return NULL;
	}
}

Type* LLVMPipe::codegen (Ast_Pointer_Type* ty) {
	PointerType* ptrTy = NULL;
	Type* baseTy = this->codegen(ty->base);
	if (PointerType::isValidElementType(baseTy)) {
		return PointerType::get(baseTy, 0);
	} else {
		outs() << "Invalid type to use in pointer: ";
		baseTy->print(outs());
		exit(1);
	}
	return ptrTy;
}

Type* LLVMPipe::codegen (Ast_Struct_Type* ty) {
	StructType* structTy;
	if (ty->attributes.size() > 0) {
		vector<Type*> attrTypes;
		for (auto &attr : ty->attributes)
			attrTypes.push_back(this->codegen(attr->type));
		structTy = StructType::create(attrTypes, ty->name);
	} else {
		structTy = StructType::create(builder.getContext(), ty->name);
	}
	this->scope->addType(ty, structTy);
	return structTy;
}
