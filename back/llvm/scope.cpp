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

using namespace llvm;
using namespace std;

class LLVMScope {
public:
	map<std::string, Value*> variables;
	LLVMScope* parent = nullptr;

	LLVMScope (LLVMScope* parent = nullptr) {
		this->parent = parent;
	}

	void addVariable (AllocaInst* allocation) {
		variables[allocation->getName()] = allocation;
	}

	void addVariables (Function* function) {
		for (auto &arg : function->args())
  			variables[arg.getName()] = &arg;
	}

	Value* get (std::string name) {
		auto it = variables.find(name);
		if (it == variables.end()) {
			if (this->parent == nullptr) return nullptr;
			else return this->parent->get(name);
		} else return variables[name];
	}

	LLVMScope* push () {
		return new LLVMScope(this);
	}

	LLVMScope* pop () {
		return this->parent;
	}
};
