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
	map<std::string, AllocaInst*> scope;

	LLVMCodeGenerator () : builder(context) { /* empty */ }

	Module* buildModule (ASTStatements* stms) {
		module = new Module("output", context);
		types = new LLVMTypeSystem(module);

		vector<Type*> params {};
		FunctionType* funcType = FunctionType::get(Type::getVoidTy(context), params, false);
		Function* mainFunc = Function::Create(funcType, Function::ExternalLinkage, "main", module);
		BasicBlock* entryBlock = BasicBlock::Create(context, "entry", mainFunc);
		builder.SetInsertPoint(entryBlock);

		this->codegen(stms);

		vector<Type*> exitArgs { Type::getInt32Ty(context) };
		funcType = FunctionType::get(Type::getVoidTy(context), exitArgs, false);
		Function* exitFunc = Function::Create(funcType, Function::ExternalLinkage, "system_exit", module);
		builder.CreateCall(exitFunc, ConstantInt::get(context, APInt(32, 0)));
		builder.CreateRetVoid();

		verifyModule(*module);
		module->print(outs(), nullptr);
		return module;
	}

	void codegen (ASTStatements* stms) {
		for(auto const& value: stms->list)
			this->codegen(value);
	}

	void codegen (ASTStatement* stm) {
		if (ASTVarDef* varDef = dynamic_cast<ASTVarDef*>(stm))
			this->codegen(varDef);
	}

	void codegen (ASTVarDef* varDef) {
		varDef->print(0);
		Type* type = this->codegen(varDef->type);
		AllocaInst* varAlloca = builder.CreateAlloca(type, nullptr, varDef->name);
		if (varDef->expression != NULL) {
			Value* val = this->codegen(varDef->expression);
			if (val != nullptr) builder.CreateStore(val, varAlloca);
			else outs() << "ERROR: Expression Value* is NULL!\n";
		}
		scope[varDef->name] = varAlloca;
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
				return builder.CreateUDiv(lhs, rhs, "div");
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

	Value* codegen (ASTConst* con) {
		switch (con->type) {
			case ASTConst::TYPE::INT:
				return ConstantInt::get(context, APInt(32, con->intValue));
			default: break;
		}
		return nullptr;
	}

	Value* codegen (ASTId* id) {
		auto it = scope.find(id->name);
		if (it == scope.end()) {
			panic("Variable " + id->name + " not found!");
			return nullptr;
		} else {
			return builder.CreateLoad(scope[id->name], "tmp");
		}
	}

	Type* codegen (ASTType* type) {
		return this->types->getType(type);
	}

	~LLVMCodeGenerator () { /* empty */ }

private:
	void* panic (std::string message) {
		outs() << "FATAR ERROR: " << message << "\n";
		exit(1);
		return nullptr;
	}
};
