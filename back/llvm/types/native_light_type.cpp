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

#include <string>
#include <vector>
#include <map>

#include "parser/parser.cpp"
#include "light_type.cpp"

using namespace llvm;

typedef Value* (*NativeCall)(IRBuilder<>*, Value*, Value*);

class NativeLiType : public LiType {
public:
	std::map<std::string, NativeCall> nativeFunctions;

	NativeLiType (std::string name) : LiType(name)
	{ /* empty */ }
};

#include "void_light_type.cpp"
#include "int32_light_type.cpp"
