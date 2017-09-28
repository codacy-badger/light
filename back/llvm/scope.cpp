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

#include "types/native_light_type.cpp"
#include "light_variable.cpp"
#include "light_function.cpp"

using namespace llvm;
using namespace std;

class LLVMScope {
public:
	LLVMScope* parent = nullptr;

	map<std::string, LiVariable*> variables;
	map<std::string, LiFunction*> functions;
	map<std::string, LiType*> types;

	LLVMScope (LLVMScope* parent = nullptr) {
		this->parent = parent;
	}

	void addVariable (LiType* type, AllocaInst* allocation) {
		LiVariable* var = new LiVariable(allocation->getName());
		var->allocation = allocation;
		var->type = type;
		this->addVariable(var);
	}

	void addVariable (IRBuilder<>* builder, std::string name, LiType* type) {
		LiVariable* var = new LiVariable(name);
		var->allocation = builder->CreateAlloca(type->llvmType, nullptr, name);
		var->type = type;
		this->addVariable(var);
	}

	void addVariable (LiVariable* var) {
		auto it = variables.find(var->name);
		if (it == variables.end())
			variables[var->name] = var;
		else cout << "Variable already exists " << var->name << "\n";
	}

	void addParameters (IRBuilder<>* builder, LiFunction* fn) {
		int i = 0;
		for (auto &arg : fn->params) {
			Type* type = arg->type->llvmType;
			AllocaInst* allocation = builder->CreateAlloca(type, 0, arg->name + ".param");

			Argument* llvmArg = std::next(fn->llvmFunction->arg_begin(), i++);
			builder->CreateStore(llvmArg, allocation);
			arg->allocation = allocation;
			variables[arg->name] = arg;
		}
	}

	LiVariable* getVariable (std::string name) {
		auto it = variables.find(name);
		if (it == variables.end()) {
			if (this->parent == nullptr) return nullptr;
			else return this->parent->getVariable(name);
		} else return variables[name];
	}

	void addType (LiType* type) {
		auto it = types.find(type->name);
		if (it == types.end())
			types[type->name] = type;
		else cout << "Type already exists " << type->name << "\n";
	}

	void addType (ASTType* def) {
		LiType* type = new LiType(def->name);
		if (def->stms != nullptr) {
			for(auto const& stm: def->stms->list) {
				if (typeid(*stm) == typeid(ASTVarDef)) {
					auto varDef = static_cast<ASTVarDef*>(stm);
					cout << "Def Type! -> " << varDef->name << "\n";
				}
			}
		} else cout << "Opaque type: " << def->name;
		this->addType(type);
	}

	void addType (std::string alias, std::string original) {
		auto it = types.find(original);
		if (it != types.end())
			types[alias] = types[original];
		else cout << "Type " << original << " not found\n";
	}

	LiType* getType (std::string name) {
		auto it = types.find(name);
		if (it != types.end())
			return types[name];
		else if (this->parent != nullptr) {
			return this->parent->getType(name);
		} else return nullptr;
	}

	LiType* getType (ASTType* type) {
		if (type == nullptr) return this->getType("void");
		return this->getType(type->name);
	}

	void addFunction (LiFunction* fn) {
		auto it = functions.find(fn->name);
		if (it == functions.end())
			functions[fn->name] = fn;
		else cout << "Function already exists " << fn->name << "\n";
	}

	LLVMScope* push () {
		return new LLVMScope(this);
	}

	LLVMScope* pop () {
		return this->parent;
	}
};
