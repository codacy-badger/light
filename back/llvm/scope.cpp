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

#include <string>
#include <map>

#include "types/native_light_type.cpp"

using namespace llvm;
using namespace std;

class LLVMScope {
public:
	LLVMScope* parent = nullptr;
	map<std::string, AllocaInst*> variables;
	map<std::string, LiType*> types;

	LLVMScope (LLVMScope* parent = nullptr) {
		this->parent = parent;
	}

	void addVariable (AllocaInst* allocation) {
		variables[allocation->getName()] = allocation;
	}

	void addParameters (IRBuilder<>* builder, Function* function) {
		for (auto &arg : function->args()) {
			Type* type = arg.getType();
			AllocaInst* alloca = builder->CreateAlloca(type, 0, arg.getName() + ".addr");
			builder->CreateStore(&arg, alloca);
			variables[arg.getName()] = alloca;
		}
	}

	AllocaInst* get (std::string name) {
		auto it = variables.find(name);
		if (it == variables.end()) {
			if (this->parent == nullptr) return nullptr;
			else return this->parent->get(name);
		} else return variables[name];
	}

	void addType (LiType* type) {
		auto it = types.find(type->name);
		if (it == types.end())
			types[type->name] = type;
		else cout << "Type already exists " << type->name << "\n";
	}

	void addType (ASTDefType* def) {
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

	LiType* getType (std::string name) {
		auto it = types.find(name);
		if (it != types.end())
			return types[name];
		else if (this->parent != nullptr) {
			return this->parent->getType(name);
		} else return nullptr;
	}

	LiType* getType (ASTType* type) {
		return this->getType(type->name);
	}

	LLVMScope* push () {
		return new LLVMScope(this);
	}

	LLVMScope* pop () {
		return this->parent;
	}
};
