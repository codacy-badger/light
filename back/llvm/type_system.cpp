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

class LLVMTypeSystem {
public:
	Module* module;
	map<ASTType*, Type*> typeCache;

	Type* Tint32 = nullptr;

	LLVMTypeSystem (Module* module) {
		this->module = module;
		Tint32 = Type::getInt32Ty(module->getContext());
	}

	Type* getNativeType (std::string name) {
		if (name == "int") return Tint32;
		else return nullptr;
	}

	std::string getNativeName (Type* type) {
		if (type == Tint32) return "int";
		else return nullptr;
	}

	std::string getName (Type* type) {
		if (!type->isStructTy()) return getNativeName(type);
		else return nullptr;
	}

	Type* getType (std::string name) {
		auto result = getNativeType(name);
		if (result == nullptr) {
			// TODO: search in the module structs
		}
		return result;
	}

	Type* getType (ASTType* type) {
		return this->getType(type->name);
	}

	std::string mangle (std::string name, FunctionType* types) {
		auto retType = types->getReturnType();
		auto result = name + "$" + getName(retType);
		for(auto const& type: types->params())
			result += "_" + getName(type);
		return result;
	}
};
