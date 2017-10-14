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

using namespace llvm;
using namespace std;

struct LLVMCodegenPrimitive {
	static Value* assign (IRBuilder<>* builder, Value* lhs, Value* rhs);
	static Value* add (IRBuilder<>* builder, Value* lhs, Value* rhs);
	static Value* sub (IRBuilder<>* builder, Value* lhs, Value* rhs);
	static Value* mul (IRBuilder<>* builder, Value* lhs, Value* rhs);
	static Value* div (IRBuilder<>* builder, Value* lhs, Value* rhs);
	static Value* neg (IRBuilder<>* builder, Value* val);
};
