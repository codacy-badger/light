#pragma once

#include <iterator>
#include <string>
#include <map>

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

#include "parser/ast.hpp"

using namespace llvm;
using namespace std;

struct LLVMScope {
	LLVMScope* parent = nullptr;
	IRBuilder<>* builder = nullptr;

	map<Ast_Variable*, AllocaInst*> variables;

	static map<Ast_Type_Definition*, Type*> types;
	static map<Ast_Function*, Function*> functions;

	LLVMScope (IRBuilder<>* builder, LLVMScope* parent = nullptr);

	void addVariable (Ast_Variable* var);
	void addVariable (Ast_Variable* var, AllocaInst* alloca);
	void addParameters (Ast_Function* fn);
	AllocaInst* getVariable (Ast_Variable* var);
	AllocaInst* getVariable (string name);

	void addType (Ast_Type_Definition* ty, Type* type);
	void addType (Ast_Type_Definition* alias, Ast_Type_Definition* original);
	Type* getType (Ast_Type_Definition* ty);

	void addFunction (Ast_Function* fn, Function* function);
	Function* getFunction (Ast_Function* fn);

	LLVMScope* push ();
	LLVMScope* pop ();
};
