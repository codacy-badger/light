#include "llvm/Support/CommandLine.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include "back/llvm/LLVMBackend.cpp"

using namespace llvm;
using namespace std;

cl::OptionCategory LightCategory("Compiler Options");

static cl::opt<std::string>
OutputFilename("o", cl::desc("Specify output file."), cl::value_desc("filename"), cl::cat(LightCategory));

void printVersion (raw_ostream& out) {
	out << "Light Compiler 0.1.0\n";
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

void printSTD (LLVMContext& context, Module* module, IRBuilder<> builder, Value* message) {
	vector<Type*> putsArgs;
	putsArgs.push_back(Type::getInt8Ty(context)->getPointerTo());
	FunctionType *functionType = FunctionType::get(Type::getVoidTy(context), putsArgs, false);
	Constant* const_func = module->getOrInsertFunction("print", functionType);
	Function* print_func = cast<Function>(const_func);

	vector<Value*> args { message };
	builder.CreateCall(print_func, args);
}

void windowsExitApplication (LLVMContext& context, Module* module, IRBuilder<> builder, int exitCode) {
	vector<Type*> putsArgs3 { Type::getInt32Ty(context) };
	Function* ExitProcessfunc = makeNativeFunction(module, "ExitProcess", Type::getVoidTy(context), putsArgs3);

	CallInst* exitResult = builder.CreateCall(ExitProcessfunc,
		ConstantInt::get(context, APInt(32, 0)));
	exitResult->setCallingConv(CallingConv::X86_StdCall);
}

Module* getHelloModule (LLVMContext& context, string message) {
	Module* module = new Module("hello", context);

	Type* Tvoid = Type::getVoidTy(context);

	vector<Type*> params;
	Function* mainFunction = makeFunction(module, "main", Tvoid, params);
	BasicBlock* block = BasicBlock::Create(context, "entry", mainFunction);
	IRBuilder<> builder(block);

	Value* messageValue = builder.CreateGlobalStringPtr(message.c_str());
	printSTD(context, module, builder, messageValue);

	windowsExitApplication(context, module, builder, 0);
	builder.CreateRet(nullptr);

	verifyModule(*module);
	return module;
}

Module* getIfElseModule (LLVMContext& context, int val) {
	Module* module = new Module("ifelse", context);
	IRBuilder<> builder(context);

	Type* Tint32 = Type::getInt32Ty(context);
	Type* Tvoid = Type::getVoidTy(context);

	vector<Type*> params;
	Function* mainFunction = makeFunction(module, "main", Tvoid, params);
	BasicBlock *entryBB = BasicBlock::Create(context, "entry", mainFunction);
	BasicBlock *thenBB = BasicBlock::Create(context, "if.then", mainFunction);
	BasicBlock *elseBB = BasicBlock::Create(context, "if.else", mainFunction);
	BasicBlock *exitBB = BasicBlock::Create(context, "exit", mainFunction);

	builder.SetInsertPoint(entryBB);

	Value* varA = builder.CreateAlloca(Tint32, nullptr, "a");
	builder.CreateStore(ConstantInt::get(context, APInt(32, 1)), varA);
	Value* varA2 = builder.CreateLoad(varA, "a2");

	Value* cond = builder.CreateICmpSGT(varA2, ConstantInt::get(context, APInt(32, 0)));
	builder.CreateCondBr(cond, thenBB, elseBB);

	builder.SetInsertPoint(thenBB);
	printSTD(context, module, builder, builder.CreateGlobalStringPtr("[ifelse] then block reached!\n"));
	builder.CreateBr(exitBB);

	builder.SetInsertPoint(elseBB);
	printSTD(context, module, builder, builder.CreateGlobalStringPtr("[ifelse] else block reached!\n"));
	builder.CreateBr(exitBB);

	builder.SetInsertPoint(exitBB);
	windowsExitApplication(context, module, builder, 0);
	builder.CreateRet(nullptr);

	return module;
}

int main (int argc, char** argv) {
	cl::SetVersionPrinter(printVersion);
	cl::HideUnrelatedOptions(LightCategory);
	cl::ParseCommandLineOptions(argc, argv);

	LLVMContext GlobalContext;
	LLVMBackend* backend = new LLVMBackend();

	Module* module = getHelloModule(GlobalContext, "[hello] I'm pickle Riiick!!\n");
	module->print(outs(), nullptr);
	auto moduleName = "test\\" + module->getModuleIdentifier() + ".obj";
	backend->writeObj(module, moduleName.c_str());

	module = getIfElseModule(GlobalContext, 12000);
	module->print(outs(), nullptr);
	moduleName = "test\\" + module->getModuleIdentifier() + ".obj";
	backend->writeObj(module, moduleName.c_str());

	delete module;

	return 0;
}
