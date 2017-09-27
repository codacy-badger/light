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

#include "native_light_type.cpp"

Value* int32__add (IRBuilder<>* builder, Value* lhs, Value* rhs) {
	return builder->CreateAdd(lhs, rhs, "add");
}

class Int32LiType : public NativeLiType {
public:
	Int32LiType (LLVMContext* context) : NativeLiType("i32") {
		this->nativeFunctions["__add"] = &int32__add;
		this->llvmType = Type::getInt32Ty(*context);
	}
};
