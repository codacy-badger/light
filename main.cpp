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

/*static cl::opt<std::string>
OutputFilename("o", cl::desc("Specify output file."), cl::value_desc("filename"), cl::cat(LightCategory));

static cl::list<std::string>
InputFilenames(cl::Positional, cl::desc("<input file>"), cl::OneOrMore, cl::cat(LightCategory));*/

void printVersion () {
	cout << "Light Compiler 0.1.0" << endl;
}

Module* makeLLVMModule(LLVMContext& context);
Function* makeFunction (Module* module, string name,
						Type* returnType, std::vector<Type*> Params);

int main() {
	LLVMContext GlobalContext;

	Module* Mod = makeLLVMModule(GlobalContext);

	verifyModule(*Mod);

	Mod->print(errs(), nullptr);

	{
		// Initialize the target registry etc.
		InitializeAllTargetInfos();
		InitializeAllTargets();
		InitializeAllTargetMCs();
		InitializeAllAsmParsers();
		InitializeAllAsmPrinters();

		auto TargetTriple = sys::getDefaultTargetTriple();
		Mod->setTargetTriple(TargetTriple);

		std::string Error;
		auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

		// Print an error and exit if we couldn't find the requested target.
		// This generally occurs if we've forgotten to initialise the
		// TargetRegistry or we have a bogus target triple.
		if (!Target) {
			errs() << Error;
			return 1;
		}

		TargetOptions opt;
		auto RM = Optional<Reloc::Model>();
		auto TheTargetMachine = Target->createTargetMachine(TargetTriple, "generic", "", opt, RM);

		Mod->setDataLayout(TheTargetMachine->createDataLayout());

		auto Filename = ".\\test\\output.obj";
		std::error_code EC;
		raw_fd_ostream dest(Filename, EC, sys::fs::F_None);

		if (EC) {
			errs() << "Could not open file: " << EC.message();
			return 1;
		}

		legacy::PassManager pass;
		auto FileType = TargetMachine::CGFT_ObjectFile;

		if (TheTargetMachine->addPassesToEmitFile(pass, dest, FileType)) {
			errs() << "TheTargetMachine can't emit a file of this type";
			return 1;
		}

		pass.run(*Mod);
		dest.flush();

		outs() << "Wrote " << Filename << "\n";
	}

	delete Mod;

	return 0;
}

Module* makeLLVMModule (LLVMContext& context) {
	Module* mod = new Module("test", context);

	Type* Tint32 = Type::getInt32Ty(context);
	Type* Tint8 = Type::getInt8Ty(context);
	Type* Tvoid = Type::getVoidTy(context);
	vector<Type*> params;
	/*params.push_back(Tint32);
	params.push_back(Tint32);
	params.push_back(Tint32);*/
	Function* mul_add = makeFunction(mod, "mainCRTStartup", Tvoid, params);

	/*Function::arg_iterator args = mul_add->arg_begin();
	Value* x = args++;
	x->setName("x2");
	Value* y = args++;
	y->setName("y2");
	Value* z = args++;
	z->setName("z2");*/

	BasicBlock* block = BasicBlock::Create(context, "entry", mul_add);
	IRBuilder<> builder(block);

	string message = "Hello LLVM!\n";
	Value* messageValue = builder.CreateGlobalStringPtr(message.c_str());

	vector<Type*> putsArgs;
	putsArgs.push_back(Tint8->getPointerTo());
	Function* print_func = makeFunction(mod, "print", Tvoid, putsArgs);
	/*GetStdHandlefunc->setDLLStorageClass(GlobalValue::DLLStorageClassTypes::DLLImportStorageClass);
	GetStdHandlefunc->setCallingConv(CallingConv::X86_StdCall);*/

	vector<Type*> putsArgs3;
	putsArgs3.push_back(Tint32);
	Function* ExitProcessfunc = makeFunction(mod, "ExitProcess", Tvoid, putsArgs3);
	ExitProcessfunc->setDLLStorageClass(GlobalValue::DLLStorageClassTypes::DLLImportStorageClass);
	ExitProcessfunc->setCallingConv(CallingConv::X86_StdCall);

	/*AllocaInst* coutVariable = builder.CreateAlloca(Tint32, 0, "retval");
	builder.CreateStore(ConstantInt::get(context, APInt(32, 0)), coutVariable, false);*/

	vector<Value*> args;
	args.push_back(messageValue);
	CallInst* writeResult = builder.CreateCall(print_func, args);
	//writeResult->setCallingConv(CallingConv::X86_StdCall);

	CallInst* exitResult = builder.CreateCall(ExitProcessfunc,
		ConstantInt::get(context, APInt(32, 0)));
	exitResult->setCallingConv(CallingConv::X86_StdCall);

	/*Value* tmp = builder.CreateBinOp(Instruction::Mul, x, y, "tmp");
	Value* tmp2 = builder.CreateBinOp(Instruction::Add, tmp, z, "tmp2");

	builder.CreateRet(tmp2);*/

	//builder.CreateRet(ConstantInt::get(context, APInt(32, 0)));
	builder.CreateRet(nullptr);

	return mod;
}

Function* makeFunction (Module* module, string name,
						Type* returnType, std::vector<Type*> Params) {
	FunctionType *FuncType = FunctionType::get(returnType, Params, false);
	Function *Func = Function::Create(FuncType, Function::ExternalLinkage, name.c_str(), module);
	return Func;
}

/*int main(int argc, char **argv) {
	cl::SetVersionPrinter(printVersion);
	cl::HideUnrelatedOptions(LightCategory);
	cl::ParseCommandLineOptions(argc, argv);

	Compiler* compiler = new Compiler();
	for (auto &filename : InputFilenames)
		compiler->addSource(filename.c_str());
	ASTStatements* stms = compiler->compile();
	delete compiler;

	//LLVMBackend* backend = new LLVMBackend();
	for (auto &stm : stms->list)
		stm->print(0);

	return 0;
}*/
