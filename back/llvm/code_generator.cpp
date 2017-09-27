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

#include "types/light_type.cpp"
#include "std/std.cpp"
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

	LLVMCodeGenerator () : builder(context) { /* empty */ }

	Module* buildModule (ASTStatements* stms) {
		module = new Module("output", context);
		scope = new LLVMScope();
		scope->addType(new VoidLiType(&context));
		scope->addType(new Int32LiType(&context));
		addStdModuleFunctions(module);

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
		else if (typeid(*stm) == typeid(ASTDefType))
			this->codegen(static_cast<ASTDefType*>(stm));
		else if (typeid(*stm) == typeid(ASTExpStatement))
			this->codegen(static_cast<ASTExpStatement*>(stm));
		else panic("Unrecognized statement?!");
	}

	void codegen (ASTVarDef* varDef) {
		LiVariable* var = new LiVariable(varDef->name);
		var->type = this->codegen(varDef->type);
		var->allocation = builder.CreateAlloca(var->type->llvmType, nullptr, varDef->name);
		if (varDef->expression != NULL) {
			Value* val = this->codegen(varDef->expression);
			if (val != nullptr)
				builder.CreateStore(val, var->allocation);
			else outs() << "ERROR: Expression Value* is NULL!\n";
		}
		scope->addVariable(var);
	}

	void codegen (ASTDefType* defType) {
		vector<Type*> structTypes;
		for(auto const& stm: defType->stms->list) {
			if (typeid(*stm) == typeid(ASTVarDef)) {
				ASTVarDef* varDef = static_cast<ASTVarDef*>(stm);
				LiType* ty = this->codegen(varDef->type);
				structTypes.push_back(ty->llvmType);
			}
		}
		StructType::create(structTypes, defType->name.c_str());
		// TODO: add this newly created type into the scope
	}

	Value* codegen (ASTExpStatement* expStm) {
		return this->codegen(expStm->exp);
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
		else if (ASTCall* call = dynamic_cast<ASTCall*>(exp))
			return this->codegen(call);
		else if (ASTAttr* attr = dynamic_cast<ASTAttr*>(exp))
			return this->codegen(attr);
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
			if (actualType->isStructTy()) {
				//return builder.CreateStructGEP(actualType, val, 0);
				Value* attrPointer = builder.CreateStructGEP(actualType, val, 0);
				return builder.CreateLoad(attrPointer, "attr." + attr->name);
			}
		} else panic("[ASTAttr::codegen] value type is not a struct!");
		return nullptr;
	}

	Value* codegen (ASTId* id) {
		LiVariable* var = scope->getVariable(id->name);
		AllocaInst* alloc = var->allocation;
		if (var == nullptr) {
			panic("Variable " + id->name + " not found!");
			return nullptr;
		} else return builder.CreateLoad(alloc, "tmp");;
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

	LiFunction* codegen (ASTFunction* func) {
		LiFunction* fn = new LiFunction(func->name);

		vector<Type*> argTypes;
		for(auto const& param: func->fnType->params) {
			LiType* argType = this->codegen(param->type);
			argTypes.push_back(argType->llvmType);

			LiVariable* param = new LiVariable(param->name);
			param->type = argType;
			fn->params.push_back(param);
		}
		LiType* retType = this->codegen(func->fnType->retType);
		FunctionType* functionType = FunctionType::get(
			retType->llvmType, argTypes, false);
		fn->returnType = retType;

		Function* function = nullptr;
		if (func->stms != nullptr) {
			int index = 0;
			function = Function::Create(functionType, Function::ExternalLinkage, func->name, module);
			for (auto& arg: function->args())
				arg.setName(func->fnType->params[index++]->name);
			fn->llvmFunction = function;

			BasicBlock* entryBlock = BasicBlock::Create(context, "entry", function);
			BasicBlock* prevBlock = builder.GetInsertBlock();
			builder.SetInsertPoint(entryBlock);

			this->scope = scope->push();
			this->scope->addParameters(&builder, fn);
			this->codegen(func->stms);
			this->scope = scope->pop();
			if (func->fnType->retType == nullptr)
				builder.CreateRetVoid();

			builder.SetInsertPoint(prevBlock);
		} else {
			Constant* constFunc = module->getOrInsertFunction(func->name, functionType);
			function = cast<Function>(constFunc);
			fn->llvmFunction = function;
		}

		verifyFunction(*function);
		this->scope->addFunction(fn);
		return fn;
	}

	LiType* codegen (ASTType* type) {
		if (type == nullptr)
			return this->scope->getType("void");
		return this->scope->getType(type);
	}

	~LLVMCodeGenerator () { /* empty */ }

private:
	Value* createCall (ASTCall* call, std::string tmpName = "") {
		Function* function = nullptr;
		if (ASTId* id = dynamic_cast<ASTId*>(call->var)) {
			function = module->getFunction(id->name);
		}
		if (function == nullptr) {
			cout << "Unknown function: ";
			call->var->print(0);
			cout << "\n";
			panic("Function* not found!");
		}

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
