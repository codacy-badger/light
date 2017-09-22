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

#include "type_system.cpp"
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
	LLVMTypeSystem* types = nullptr;
	stack<Function*> functionStack;
	LLVMScope* scope;

	LLVMCodeGenerator () : builder(context) { /* empty */ }

	Module* buildModule (ASTStatements* stms) {
		module = new Module("output", context);
		types = new LLVMTypeSystem(module);
		scope = new LLVMScope();

		this->codegen(stms);

		verifyModule(*module);
		module->print(outs(), nullptr);
		return module;
	}

	void codegen (ASTStatements* stms) {
		for(auto const& value: stms->list)
			this->codegen(value);
	}

	void codegen (ASTStatement* stm) {
		if (typeid(*stm) == typeid(ASTVarDef))
			this->codegen(static_cast<ASTVarDef*>(stm));
		else if (typeid(*stm) == typeid(ASTStatements))
			this->codegen(static_cast<ASTStatements*>(stm));
		else if (typeid(*stm) == typeid(ASTFunction))
			this->codegen(static_cast<ASTFunction*>(stm));
		else if (typeid(*stm) == typeid(ASTReturn))
			this->codegen(static_cast<ASTReturn*>(stm));
		else if (typeid(*stm) == typeid(ASTCallStatement))
			this->codegen(static_cast<ASTCallStatement*>(stm));
		else if (typeid(*stm) == typeid(ASTDefType))
			this->codegen(static_cast<ASTDefType*>(stm));
		else panic("Unrecognized statement?!");
	}

	void codegen (ASTVarDef* varDef) {
		Type* type = this->codegen(varDef->type);
		AllocaInst* varAlloca = builder.CreateAlloca(type, nullptr, varDef->name);
		if (varDef->expression != NULL) {
			Value* val = this->codegen(varDef->expression);
			if (val != nullptr) builder.CreateStore(val, varAlloca);
			else outs() << "ERROR: Expression Value* is NULL!\n";
		}
		scope->addVariable(varAlloca);
	}

	void codegen (ASTDefType* defType) {
		vector<Type*> structTypes;
		for(auto const& stm: defType->stms->list) {
			if (typeid(*stm) == typeid(ASTVarDef)) {
				ASTVarDef* varDef = static_cast<ASTVarDef*>(stm);
				structTypes.push_back(this->codegen(varDef->type));
			}
		}
		StructType::create(structTypes, defType->name.c_str());
	}

	Value* codegen (ASTCallStatement* callStm) {
		return createCall(callStm->call);
	}

	Value* codegen (ASTCall* call) {
		return createCall(call, "fnCall");
	}

	Value* codegen (ASTExpression* exp) {
		if (ASTBinop* binop = dynamic_cast<ASTBinop*>(exp))
			return this->codegen(binop);
		else if (ASTUnop* unop = dynamic_cast<ASTUnop*>(exp))
			return this->codegen(unop);
		else if (ASTConst* con = dynamic_cast<ASTConst*>(exp))
			return this->codegen(con);
		else if (ASTId* id = dynamic_cast<ASTId*>(exp))
			return this->codegen(id);
		else if (ASTCall* call = dynamic_cast<ASTCall*>(exp))
			return this->codegen(call);
		else if (ASTAttr* attr = dynamic_cast<ASTAttr*>(exp))
			return this->codegen(attr);
		else return nullptr;
	}

	Value* codegen (ASTBinop* binop) {
		Value* lhs = this->codegen(binop->lhs);
		Value* rhs = this->codegen(binop->rhs);
		switch (binop->op) {
			case ASTBinop::OPS::ADD:
				return builder.CreateAdd(lhs, rhs, "add");
			case ASTBinop::OPS::SUB:
				return builder.CreateSub(lhs, rhs, "sub");
			case ASTBinop::OPS::MUL:
				return builder.CreateMul(lhs, rhs, "mul");
			case ASTBinop::OPS::DIV:
				return builder.CreateSDiv(lhs, rhs, "div");
			default: break;
		}
		return nullptr;
	}

	Value* codegen (ASTUnop* unop) {
		Value* val = this->codegen(unop->expression);
		switch (unop->op) {
			case ASTUnop::OPS::NEG:
				return builder.CreateNeg(val, "neg");
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
			if (actualType->isStructTy())
				return builder.CreateStructGEP(actualType, val, 0);
		} else panic("[ASTAttr::codegen] value type is not a struct!");
		//return nullptr;
		return ConstantInt::get(context, APInt(32, 51));
	}

	Value* codegen (ASTId* id) {
		auto alloca = scope->get(id->name);
		if (alloca == nullptr) {
			panic("Variable " + id->name + " not found!");
			return nullptr;
		} else return alloca;
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

	void codegen (ASTReturn* ret) {
		if (ret->expression != nullptr) {
			Value* retValue = this->codegen(ret->expression);
			builder.CreateRet(retValue);
		} else builder.CreateRetVoid();
	}

	Function* codegen (ASTFunction* func) {
		vector<Type*> argTypes;
		for(auto const& param: func->fnType->params) {
			Type* argType = this->codegen(param->type);
			argTypes.push_back(argType);
		}
		FunctionType* functionType = FunctionType::get(
			this->codegen(func->fnType->retType), argTypes, false);

		Function* function = nullptr;
		if (func->stms != nullptr) {
			int index = 0;
			function = Function::Create(functionType, Function::ExternalLinkage, func->name, module);
			for (auto& arg: function->args())
				arg.setName(func->fnType->params[index++]->name);

			BasicBlock* entryBlock = BasicBlock::Create(context, "entry", function);
			BasicBlock* prevBlock = builder.GetInsertBlock();
			builder.SetInsertPoint(entryBlock);

			this->scope = scope->push();
			this->scope->addVariables(function, &builder);
			this->codegen(func->stms);
			this->scope = scope->pop();
			if (func->fnType->retType == nullptr)
				builder.CreateRetVoid();

			builder.SetInsertPoint(prevBlock);
		} else {
			Constant* constFunc = module->getOrInsertFunction(func->name, functionType);
			function = cast<Function>(constFunc);
		}
		verifyFunction(*function);
		return function;
	}

	Type* codegen (ASTType* type) {
		if (type == nullptr) return Type::getVoidTy(context);
		return this->types->getType(type);
	}

	~LLVMCodeGenerator () { /* empty */ }

private:
	Value* createCall (ASTCall* call, std::string tmpName = "") {
		Function* function = module->getFunction(call->name);
		if (function == nullptr)
			panic("Function* not found -> " + call->name);
		vector<Value*> params;
		for(auto const& param: call->params)
			params.push_back(this->codegen(param));
		return builder.CreateCall(function, params, tmpName);
	}

	void* panic (std::string message) {
		outs() << "FATAR ERROR: " << message << "\n";
		exit(1);
		return nullptr;
	}
};
