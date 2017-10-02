#pragma once

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "primitive_op.cpp"
#include "scope.cpp"

#include <string>
#include <ostream>
#include <iostream>
#include <stack>
#include <map>

using namespace llvm;
using namespace std;

class LLVMCodeGenerator {
public:
	LLVMContext context;
	IRBuilder<> builder;

	Module* module = nullptr;
	stack<Function*> functionStack;
	LLVMScope* scope;

	LLVMCodeGenerator () : builder(context) {
		scope = new LLVMScope(&builder);
		scope->addType(ASTPrimitiveType::_void, Type::getVoidTy(context));
		scope->addType(ASTPrimitiveType::_i32, Type::getInt32Ty(context));
		scope->addType(ASTPrimitiveType::_int, ASTPrimitiveType::_i32);
	}

	Module* buildModule (ASTStatements* stms) {
		module = new Module("output", context);

		this->codegen(stms);

		verifyModule(*module);
		module->print(outs(), nullptr);
		return module;
	}

	Value* codegen (ASTStatements* stms) {
		for(auto const& value: stms->list)
			this->codegen(value);
		return nullptr;
	}

	Value* codegen (ASTStatement* stm) {
		if 		(auto obj = dynamic_cast<ASTVariable*>(stm)) 	return codegen(obj, true);
		else if (auto obj = dynamic_cast<ASTStatements*>(stm)) 	return codegen(obj);
		else if (auto obj = dynamic_cast<ASTFunction*>(stm)) 	return codegen(obj);
		else if (auto obj = dynamic_cast<ASTReturn*>(stm)) 		return codegen(obj);
		else if (auto obj = dynamic_cast<ASTType*>(stm)) {
			codegen(obj);
			return nullptr;
		} else if (auto obj = dynamic_cast<ASTExpression*>(stm)) 	return codegen(obj);
		else {
			std::string msg = "Unrecognized statement?! -> ";
			msg += typeid(*stm).name();
			msg += "\n";
			panic(msg.c_str());
			return nullptr;
		}
	}

	Value* codegen (ASTVariable* varDef, bool alloca = false) {
		if (alloca) {
			auto type = this->codegen(varDef->type);
			auto alloca = builder.CreateAlloca(type, nullptr, varDef->name);
			if (varDef->expression != NULL) {
				Value* val = this->codegen(varDef->expression);
				builder.CreateStore(val, alloca);
			}
			this->scope->addVariable(varDef, alloca);
			return alloca;
		} else {
			return this->scope->getVariable(varDef);
		}
	}

	Type* codegen (ASTType* ty) {
		auto type = this->scope->getType(ty);
		if (type != nullptr) return type;
		else {
			if (auto obj = dynamic_cast<ASTStructType*>(ty)) return codegen(obj);
			else {
				std::string msg = "Unrecognized type struct?! -> ";
				msg += typeid(*ty).name();
				msg += "\n";
				panic(msg.c_str());
				return nullptr;
			}
		}
	}

	Type* codegen (ASTStructType* ty) {
		StructType* structTy = nullptr;
		if (ty->attrs.size() > 0) {
			vector<Type*> attrTypes;
			for (auto &attr : ty->attrs)
				attrTypes.push_back(this->codegen(attr->type));
			structTy = StructType::create(attrTypes, ty->name);
		} else {
			structTy = StructType::create(builder.getContext(), ty->name);
		}
		this->scope->addType(ty, structTy);
		return structTy;
	}

	Value* codegen (ASTCall* call) {
		return createCall(call, "fnCall");
	}

	Value* codegen (ASTExpression* exp) {
		if 		(auto binop = dynamic_cast<ASTBinop*>(exp))   return codegen(binop);
		else if (auto unop  = dynamic_cast<ASTUnop*>(exp))    return codegen(unop);
		else if (auto con   = dynamic_cast<ASTConst*>(exp))   return codegen(con);
		else if (auto call  = dynamic_cast<ASTCall*>(exp)) 	  return codegen(call);
		else if (auto attr  = dynamic_cast<ASTAttr*>(exp)) 	  return codegen(attr);
		else if (auto var   = dynamic_cast<ASTVariable*>(exp)) {
			return builder.CreateLoad(codegen(var), var->name);
		} else {
			std::string msg = "Unrecognized expression?! -> ";
			msg += typeid(*exp).name();
			msg += "\n";
			panic(msg.c_str());
			return nullptr;
		}
	}

	Value* codegen (ASTBinop* binop) {
		Value* lhs = this->codegen(binop->lhs);
		Value* rhs = this->codegen(binop->rhs);
		switch (binop->op) {
			case ASTBinop::OP::ADD:
				return PrimitiveOP::add(&builder, lhs, rhs);
			case ASTBinop::OP::SUB:
				return PrimitiveOP::sub(&builder, lhs, rhs);
			case ASTBinop::OP::MUL:
				return PrimitiveOP::mul(&builder, lhs, rhs);
			case ASTBinop::OP::DIV:
				return PrimitiveOP::div(&builder, lhs, rhs);
			default: break;
		}
		return nullptr;
	}

	Value* codegen (ASTUnop* unop) {
		Value* val = this->codegen(unop->exp);
		switch (unop->op) {
			case ASTUnop::OP::NEG:
				return PrimitiveOP::neg(&builder, val);
			default: break;
		}
		return nullptr;
	}

	Value* codegen (ASTAttr* attr) {
		Value* val = this->codegen(attr->exp);
		Type* ty = val->getType();
		if (ty->isPointerTy()) {
			PointerType* pTy = static_cast<PointerType*>(ty);
			Type* actualType = pTy->getElementType();
			if (actualType->isStructTy()) {
				//return builder.CreateStructGEP(actualType, val, 0);
				Value* attrPointer = builder.CreateStructGEP(actualType, val, 0);
				return builder.CreateLoad(attrPointer, "attr." + attr->name);
			}
		} else panic("[ASTAttr::codegen] value type is not a struct!");
		return nullptr;
	}

	Value* codegen (ASTConst* con) {
		switch (con->type) {
			case ASTConst::TYPE::INT:
				return ConstantInt::get(context, APInt(32, con->intValue));
			case ASTConst::TYPE::STRING:
				return builder.CreateGlobalStringPtr(con->stringValue);
			default: break;
		}
		return nullptr;
	}

	Value* codegen (ASTReturn* ret) {
		if (ret->exp != nullptr) {
			Value* retValue = this->codegen(ret->exp);
			return builder.CreateRet(retValue);
		} else return builder.CreateRetVoid();
	}

	Function* codegen (ASTFunction* fn) {
		auto function = module->getFunction(fn->name);
		if (function != nullptr) return function;

		vector<Type*> argTypes;
		for(auto const& param: fn->type->params)
			argTypes.push_back(this->scope->getType(param->type));
		Type* retType = this->scope->getType(fn->type->retType);
		auto fnType = FunctionType::get(retType, argTypes, false);
		auto constValue = module->getOrInsertFunction(fn->name, fnType);
		function = static_cast<Function*>(constValue);

		int index = 0;
		for (auto& arg: function->args())
			arg.setName(fn->type->params[index++]->name);
		this->scope->addFunction(fn, function);

		if (fn->stms != nullptr) {
			BasicBlock* entryBlock = BasicBlock::Create(context, "entry", function);
			BasicBlock* prevBlock = builder.GetInsertBlock();
			builder.SetInsertPoint(entryBlock);

			this->scope = scope->push();
			this->scope->addParameters(fn);
			this->codegen(fn->stms);
			this->scope = scope->pop();
			if (fn->type->retType == ASTPrimitiveType::_void
				&& !entryBlock->back().isTerminator())
				builder.CreateRetVoid();

			builder.SetInsertPoint(prevBlock);
		}

		verifyFunction(*function);
		return function;
	}

	~LLVMCodeGenerator () { /* empty */ }

private:
	Value* createCall (ASTCall* call, std::string tmpName = "") {
		Function* function = nullptr;
		if (auto fn = dynamic_cast<ASTFunction*>(call->var)) {
			function = this->scope->getFunction(fn);
			if (function == nullptr)
				function = this->codegen(fn);
		}
		if (function == nullptr) {
			panic("Function* not found!");
		}

		vector<Value*> params;
		for(auto const& param: call->params)
			params.push_back(this->codegen(param));
		return builder.CreateCall(function, params, tmpName);
	}

	void* panic (std::string message) {
		outs() << "ERROR: " << message << "\n";
		exit(1);
		return nullptr;
	}
};
