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

	LLVMCodeGenerator () : builder(context) {
		scope = new LLVMScope(&builder);
		scope->addType(ASTPrimitiveType::_void, Type::getVoidTy(context));
		scope->addType(ASTPrimitiveType::_i32, Type::getInt32Ty(context));
		scope->addType(ASTPrimitiveType::_int, ASTPrimitiveType::_i32);
	}

	Module* buildModule (ASTStatements* stms) {
		module = new Module("output", context);
		addStdModuleFunctions(module);

		this->codegen(stms);

		verifyModule(*module);
		//module->print(outs(), nullptr);
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
		else if (typeid(*stm) == typeid(ASTType))
			this->codegen(static_cast<ASTType*>(stm));
		else if (typeid(*stm) == typeid(ASTExpStatement))
			this->codegen(static_cast<ASTExpStatement*>(stm));
		else panic("Unrecognized statement?!");
	}

	void codegen (ASTVarDef* varDef) {
		auto type = this->scope->getType(varDef->type);
		auto alloca = builder.CreateAlloca(type, nullptr, varDef->name);
		if (varDef->expression != NULL) {
			Value* val = this->codegen(varDef->expression);
			builder.CreateStore(val, alloca);
		}
		this->scope->addVariable(varDef, alloca);
	}

	void codegen (ASTType* defType) {
		vector<Type*> structTypes;
		for(auto const& stm: defType->stms->list) {
			if (typeid(*stm) == typeid(ASTVarDef)) {
				ASTVarDef* varDef = static_cast<ASTVarDef*>(stm);
				auto type = this->scope->getType(varDef->type);
				structTypes.push_back(type);
			}
		}
		auto type = StructType::create(structTypes, defType->name.c_str());
		this->scope->addType(defType, type);
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
		else if (ASTPointer* id = dynamic_cast<ASTPointer*>(exp))
			return this->codegen(id);
		else return nullptr;
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

	Value* codegen (ASTPointer* id) {
		AllocaInst* alloca = this->scope->getVariable(id->name);
		if (alloca == nullptr) {
			panic("Variable " + id->name + " not found!");
			return nullptr;
		} else return builder.CreateLoad(alloca, "tmp");;
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
		if (ret->exp != nullptr) {
			Value* retValue = this->codegen(ret->exp);
			builder.CreateRet(retValue);
		} else builder.CreateRetVoid();
	}

	Function* codegen (ASTFunction* fn) {
		vector<Type*> argTypes;
		for(auto const& param: fn->type->params) {
			Type* type = this->scope->getType(param->type);
			argTypes.push_back(type);
		}
		Type* retType = this->scope->getType(fn->type->retType);
		auto fnType = FunctionType::get(retType, argTypes, false);

		int index = 0;
		auto function = Function::Create(fnType, Function::ExternalLinkage, fn->name, module);
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
			if (fn->type->retType == nullptr)
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
		if (ASTPointer* id = dynamic_cast<ASTPointer*>(call->var)) {
			function = module->getFunction(id->name);
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
		outs() << "FATAR ERROR: " << message << "\n";
		exit(1);
		return nullptr;
	}
};
