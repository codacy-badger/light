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

#include "llvm_native_types.cpp"

#include <string>
#include <ostream>
#include <iostream>
#include <map>

using namespace llvm;
using namespace std;

class LLVMCodeGenerator {
public:
	LLVMContext context;
	IRBuilder<> builder;

	Module* module = nullptr;

	LLVMCodeGenerator () : builder(context)
	{ /* empty */ }

	Module* buildModule (ASTStatements* stms) {
		module = new Module("output", context);
		LLVMNativeTypes::initNativeTypes(module);
		this->codegen(stms);
		verifyModule(*module);
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
		this->codegen(varDef->type);
		varDef->print(0);
	}

	Type* codegen (ASTType* type) {
		type->print(0);
		return Type::getInt32Ty(context);
	}

	~LLVMCodeGenerator () { /* empty */ }
};
