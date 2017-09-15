#include "llvm/Support/CommandLine.h"
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

#include "backend/llvm_backend.cpp"
#include "compiler.cpp"

#include <cstdarg>

using namespace llvm;
using namespace std;

cl::OptionCategory LightCategory("Compiler Options");

static cl::opt<std::string>
OutputFilename("o", cl::desc("Specify output file."), cl::value_desc("filename"), cl::cat(LightCategory));

/*static cl::list<std::string>
InputFilenames(cl::Positional, cl::desc("<input file>"), cl::OneOrMore, cl::cat(LightCategory));*/

void printVersion (raw_ostream& out) {
	out << "Light Compiler 0.1.0\n";
}

void toOBJFile (Module* module) {
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
	if (!OutputFilename.empty()) filename = OutputFilename.c_str();

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

Function* makeFunction (Module* module, string name, Type* returnType, std::vector<Type*> params) {
	FunctionType *functionType = FunctionType::get(returnType, params, false);
	Function *function = Function::Create(functionType, Function::ExternalLinkage, name.c_str(), module);
	return function;
}

Function* makeNativeFunction (Module* module, string name, Type* returnType, std::vector<Type*> params) {
	Function *function = makeFunction(module, name, returnType, params);
	function->setDLLStorageClass(GlobalValue::DLLStorageClassTypes::DLLImportStorageClass);
	function->setCallingConv(CallingConv::X86_StdCall);
	return function;
}

Module* makeLLVMModule (LLVMContext& context) {
	Module* module = new Module("", context);

	Type* Tint32 = Type::getInt32Ty(context);
	Type* Tint8 = Type::getInt8Ty(context);
	Type* Tvoid = Type::getVoidTy(context);

	vector<Type*> params;
	Function* mainFunction = makeFunction(module, "main", Tvoid, params);
	BasicBlock* block = BasicBlock::Create(context, "entry", mainFunction);
	IRBuilder<> builder(block);

	string message = "\nI'm pickle Riiick!!\n\n";
	Value* messageValue = builder.CreateGlobalStringPtr(message.c_str());

	vector<Type*> putsArgs;
	putsArgs.push_back(Tint8->getPointerTo());
	Function* print_func = makeFunction(module, "print", Tvoid, putsArgs);

	vector<Type*> putsArgs3 { Tint32 };
	Function* ExitProcessfunc = makeNativeFunction(module, "ExitProcess", Tvoid, putsArgs3);

	vector<Value*> args { messageValue };
	builder.CreateCall(print_func, args);

	CallInst* exitResult = builder.CreateCall(ExitProcessfunc,
		ConstantInt::get(context, APInt(32, 0)));
	exitResult->setCallingConv(CallingConv::X86_StdCall);

	builder.CreateRet(nullptr);

	verifyModule(*module);
	return module;
}

int main (int argc, char** argv) {
	cl::SetVersionPrinter(printVersion);
	cl::HideUnrelatedOptions(LightCategory);
	cl::ParseCommandLineOptions(argc, argv);

	InitializeAllTargetInfos();
	InitializeAllTargets();
	InitializeAllTargetMCs();
	InitializeAllAsmParsers();
	InitializeAllAsmPrinters();

	LLVMContext GlobalContext;

	Module* module = makeLLVMModule(GlobalContext);
	module->print(errs(), nullptr);
	toOBJFile(module);
	delete module;

	return 0;
}
