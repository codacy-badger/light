#pragma once

#include "back/llvm/llvm_obj_writter.hpp"

void LLVMObjWritter::writeObj (Module* module, const char* filepath) {
	if (!init) LLVMObjWritter::initLLVM();

	auto TargetTriple = sys::getDefaultTargetTriple();
	module->setTargetTriple(TargetTriple);

	std::string Error;
	auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
	if (!Target) errs() << Error;

	TargetOptions opt;
	auto RM = Optional<Reloc::Model>();
	auto TheTargetMachine = Target->createTargetMachine(TargetTriple, "generic", "", opt, RM);
	module->setDataLayout(TheTargetMachine->createDataLayout());

	auto filename = module->getModuleIdentifier();
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
}

void LLVMObjWritter::initLLVM () {
	InitializeAllTargetInfos();
	InitializeAllTargets();
	InitializeAllTargetMCs();
	InitializeAllAsmParsers();
	InitializeAllAsmPrinters();
	LLVMObjWritter::init = true;
}

bool LLVMObjWritter::init = false;
