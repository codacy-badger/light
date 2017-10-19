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
#include "llvm/Support/SourceMgr.h"

#include <string>

#include "parser/pipes.hpp"
#include "codegen_primitive.hpp"
#include "llvm_obj_writter.hpp"

struct LLVMPipe : Pipe {
	LLVMContext context;
	IRBuilder<> builder;
	Module* module;

	Function* currentFunction;
	AllocaInst* currentReturnVal;
	bool functionHasReturned;

	LLVMPipe (const char* output);

	void onStatement(Ast_Statement* stm);
	void onFinish ();

	Value* codegen (Ast_Statement* stm);
	Value* codegen (Ast_Block* stms);
	Value* codegen (Ast_Return* ret);
	Type* codegen (Ast_Type_Definition* ty);
	Value* codegen (Ast_Declaration* varDef, bool alloca = false);

	Value* codegen (Ast_Expression* exp);
	Value* codegen (Ast_Function* fn);
	Type* codegen (Ast_Type_Instance* ty);
	Value* codegen (Ast_Binary* binop);
	Value* codegen (Ast_Unary* unop);
	Value* codegen (Ast_Ident* ident);
	Value* codegen (Ast_Literal* con);
	Value* codegen (Ast_Function_Call* call);
};
