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

using namespace llvm;

void addFunction (Module* module, std::string name, Type* retType, std::vector<Type*> params) {
	FunctionType *functionType = FunctionType::get(retType, params, false);
	module->getOrInsertFunction(name.c_str(), functionType);
}

#include "sys.cpp"
#include "print.cpp"

// TODO: replace this by an actual "header" file for STD
void addStdModuleFunctions (Module* module) {
	addSysModuleFunctions(module);
	addPrintModuleFunctions(module);
}
