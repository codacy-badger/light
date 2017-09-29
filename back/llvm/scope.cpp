#pragma once

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
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

#include <iterator>
#include <string>
#include <map>

using namespace llvm;
using namespace std;

class LLVMScope {
public:
	LLVMScope* parent = nullptr;
	IRBuilder<>* builder = nullptr;

	map<ASTVariable*, AllocaInst*> variables;
	map<ASTFunction*, Function*> functions;
	map<ASTType*, Type*> types;

	LLVMScope (IRBuilder<>* builder, LLVMScope* parent = nullptr) {
		this->builder = builder;
		this->parent = parent;
	}

	void addVariable (ASTVariable* var) {
		auto type = this->getType(var->type);
		auto alloca = builder->CreateAlloca(type, nullptr, var->name);
		this->addVariable(var, alloca);
	}

	void addVariable (ASTVariable* var, AllocaInst* alloca) {
		auto it = variables.find(var);
		if (it == variables.end()) variables[var] = alloca;
		else cout << "Variable already exists " << var->name << "\n";
	}

	void addParameters (ASTFunction* fn) {
		int i = 0;
		auto function = this->getFunction(fn);
		for (auto &param : fn->type->params) {
			auto type = this->getType(param->type);
			auto alloca = builder->CreateAlloca(type, 0, param->name + ".arg");

			Argument* llvmArg = std::next(function->arg_begin(), i++);
			builder->CreateStore(llvmArg, alloca);

			this->addVariable(param, alloca);
		}
	}

	AllocaInst* getVariable (ASTVariable* var) {
		auto it = variables.find(var);
		if (it == variables.end()) return nullptr;
		else return variables[var];
	}

	AllocaInst* getVariable (string name) {
		for(auto it = variables.begin(); it != variables.end(); ++it) {
			if (it->first->name == name) return it->second;
		}
		cout << "Variable " << name << " not found\n";
		return nullptr;
	}

	void addType (ASTType* ty, Type* type) {
		auto it = types.find(ty);
		if (it == types.end()) types[ty] = type;
		//TODO: print the name of the type (virtual function?)
		else cout << "Type already exists \n";
	}

	void addType (ASTType* alias, ASTType* original) {
		auto it = types.find(original);
		if (it != types.end()) types[alias] = types[original];
		else cout << "Type  not found\n";
	}

	Type* getType (ASTType* ty) {
		if (ty == nullptr) return Type::getVoidTy(builder->getContext());
		auto it = types.find(ty);
		if (it != types.end())
			return types[ty];
		else if (this->parent != nullptr) {
			return this->parent->getType(ty);
		} else return nullptr;
	}

	void addFunction (ASTFunction* fn, Function* function) {
		auto it = functions.find(fn);
		if (it == functions.end()) functions[fn] = function;
		else cout << "Function already exists " << fn->name << "\n";
	}

	Function* getFunction (ASTFunction* fn) {
		auto it = functions.find(fn);
		if (it != functions.end())
			return functions[fn];
		else if (this->parent != nullptr) {
			return this->parent->getFunction(fn);
		} else return nullptr;
	}

	LLVMScope* push () {
		return new LLVMScope(builder, this);
	}

	LLVMScope* pop () {
		if (this->parent != nullptr) return this->parent;
		else {
			cout << "ERROR: pop of global scope!";
			return this;
		}
	}
};
