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

#include "parser/ast/ast.hpp"
#include "code_generator.cpp"

using namespace llvm;
using namespace std;

class LLVMBackend {
public:
	LLVMCodeGenerator* generator;

	LLVMBackend () {
		InitializeAllTargetInfos();
		InitializeAllTargets();
		InitializeAllTargetMCs();
		InitializeAllAsmParsers();
		InitializeAllAsmPrinters();
	}

	void writeObj (ASTStatements* stms, const char* filepath = nullptr) {
		LLVMCodeGenerator* generator = new LLVMCodeGenerator();
		Module* module = generator->buildModule(stms);
		this->writeObj(module, filepath);
		delete module;
		delete generator;
	}

	void writeObj (Module* module, const char* filepath = nullptr) {
		auto TargetTriple = sys::getDefaultTargetTriple();
		module->setTargetTriple(TargetTriple);

		std::string Error;
		auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
		if (!Target) errs() << Error;

		TargetOptions opt;
		auto RM = Optional<Reloc::Model>();
		auto TheTargetMachine = Target->createTargetMachine(TargetTriple, "generic", "", opt, RM);
		module->setDataLayout(TheTargetMachine->createDataLayout());

		auto filename = module->getModuleIdentifier() + ".obj";
		if (filepath != nullptr && filepath[0] != '\0')
			filename = filepath;

		std::error_code EC;
		raw_fd_ostream dest(filename, EC, sys::fs::F_None);
		if (EC) errs() << "Could not open file: " << EC.message();

		legacy::PassManager pass;
		if (TheTargetMachine->addPassesToEmitFile(pass, dest, TargetMachine::CGFT_ObjectFile))
			errs() << "TheTargetMachine can't emit a file of this type";
		pass.run(*module);
		dest.flush();

		outs() << "Wrote '" << filename << "'\n";
	}

	~LLVMBackend () { /* empty */ }
};
