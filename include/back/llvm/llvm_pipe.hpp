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
#include "llvm_scope.hpp"

struct LLVMPipe : Pipe {
	LLVMContext context;
	IRBuilder<> builder;
	Module* module;
	LLVMScope* scope;

	Function* currentFunction;
	AllocaInst* currentReturnVal;
	bool functionHasReturned;

	LLVMPipe (std::string output);

	void onFunction (Ast_Function* fn);
	void onType (Ast_Type_Definition* ty);
	void onFinish ();

	Value* codegen (Ast_Statement* stm);
	Value* codegen (Ast_Block* stms);
	Value* codegen (Ast_Variable* varDef, bool alloca = false);
	Value* codegen (Ast_Return* ret);
	Value* codegen (Ast_Expression* exp);
	Value* codegen (AST_Binary* binop);
	Value* codegen (AST_Unary* unop);
	Value* codegen (Ast_Literal* con);
	Value* codegen (Ast_Function_Call* call);
	Value* codegen(AST_Memory* mem, bool autoDeref = false);
	Value* codegen (AST_Ref* ref);
	Value* codegen (Ast_Attribute* attr);
	Value* codegen (Ast_Deref* deref);
	Type* codegen (Ast_Type_Definition* ty);
	Type* codegen (Ast_Pointer_Type* ty);
	Type* codegen (Ast_Struct_Type* ty);
};
