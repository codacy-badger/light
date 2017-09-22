#include "llvm/Support/CommandLine.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include "parser/parser.cpp"
#include "back/llvm/backend.cpp"

using namespace llvm;
using namespace std;

cl::OptionCategory LightCategory("Compiler Options");

static cl::opt<string>
OutputFilename("o", cl::desc("Specify output file."), cl::value_desc("filename"), cl::cat(LightCategory));

static cl::list<string>
InputFilenames(cl::Positional, cl::desc("<input files>"), cl::OneOrMore);

void printVersion (raw_ostream& out) {
	out << "Light Compiler 0.1.0\n";
}

Function* makeFunction (Module* module, string name, Type* returnType, std::vector<Type*> params) {
	FunctionType *functionType = FunctionType::get(returnType, params, false);
	Function *function = Function::Create(functionType, Function::ExternalLinkage, name.c_str(), module);
	//function->addFnAttr(Attribute::AttrKind::AlwaysInline);
	return function;
}

void std_print (LLVMContext& context, Module* module, IRBuilder<> builder, Value* message) {
	vector<Type*> putsArgs;
	putsArgs.push_back(Type::getInt8Ty(context)->getPointerTo());
	FunctionType *functionType = FunctionType::get(Type::getVoidTy(context), putsArgs, false);
	Constant* const_func = module->getOrInsertFunction("print", functionType);
	Function* print_func = cast<Function>(const_func);

	vector<Value*> args { message };
	builder.CreateCall(print_func, args);
}

void std_print_i32 (LLVMContext& context, Module* module, IRBuilder<> builder, Value* number) {
	vector<Type*> putsArgs;
	putsArgs.push_back(Type::getInt32Ty(context));
	FunctionType *functionType = FunctionType::get(Type::getVoidTy(context), putsArgs, false);
	Constant* const_func = module->getOrInsertFunction("print_i32", functionType);
	Function* print_func = cast<Function>(const_func);

	vector<Value*> args { number };
	builder.CreateCall(print_func, args);
}

void std_exit (LLVMContext& context, Module* module, IRBuilder<> builder, int exitCode) {
	vector<Type*> putsArgs3 { Type::getInt32Ty(context) };
	Function* ExitProcessfunc = makeFunction(module, "system_exit", Type::getVoidTy(context), putsArgs3);
	builder.CreateCall(ExitProcessfunc, ConstantInt::get(context, APInt(32, exitCode)));
	builder.CreateRetVoid();
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
	std_print(context, module, builder, builder.CreateGlobalStringPtr("[ifelse] then block reached!\n"));
	builder.CreateStore(ConstantInt::get(context, APInt(32, 42)), varA);
	builder.CreateBr(exitBB);

	builder.SetInsertPoint(elseBB);
	std_print(context, module, builder, builder.CreateGlobalStringPtr("[ifelse] else block reached!\n"));
	builder.CreateStore(ConstantInt::get(context, APInt(32, -42)), varA);
	builder.CreateBr(exitBB);

	builder.SetInsertPoint(exitBB);
	std_print(context, module, builder, builder.CreateGlobalStringPtr("[ifelse] value -> "));
	std_print_i32(context, module, builder, builder.CreateLoad(varA, "a2"));
	std_print(context, module, builder, builder.CreateGlobalStringPtr("\n"));
	std_exit(context, module, builder, 0);

	return module;
}

Module* getForModule (LLVMContext& context, int val) {
	Module* module = new Module("for", context);
	IRBuilder<> builder(context);

	Type* Tint32 = Type::getInt32Ty(context);
	Type* Tint8 = Type::getInt8Ty(context);
	Type* Tvoid = Type::getVoidTy(context);

	vector<Type*> params;
	Function* mainFunction = makeFunction(module, "main", Tvoid, params);
	BasicBlock *entryBB = BasicBlock::Create(context, "entry", mainFunction);
	BasicBlock *condBB = BasicBlock::Create(context, "for.cond", mainFunction);
	BasicBlock *bodyBB = BasicBlock::Create(context, "for.body", mainFunction);
	BasicBlock *incBB = BasicBlock::Create(context, "for.inc", mainFunction);
	BasicBlock *exitBB = BasicBlock::Create(context, "exit", mainFunction);

	builder.SetInsertPoint(entryBB);
	Value* varA = builder.CreateAlloca(Tint32, nullptr, "a");
	Value* varI = builder.CreateAlloca(Tint8, nullptr, "i");
	builder.CreateStore(ConstantInt::get(context, APInt(32, 0)), varA);
	builder.CreateStore(ConstantInt::get(context, APInt(8, 0)), varI);
	builder.CreateBr(condBB);

	builder.SetInsertPoint(condBB);
	Value* varIv = builder.CreateLoad(varI, "i2");
	Value* cond = builder.CreateICmpSLT(varIv, ConstantInt::get(context, APInt(8, val)));
	builder.CreateCondBr(cond, bodyBB, exitBB);

	builder.SetInsertPoint(bodyBB);
	Value* varAv = builder.CreateLoad(varA, "a2");
	Value* addA = builder.CreateAdd(varAv, ConstantInt::get(context, APInt(32, 2)), "add");
	std_print(context, module, builder, builder.CreateGlobalStringPtr("[for] "));
	std_print_i32(context, module, builder, builder.CreateLoad(varI, "i"));
	std_print(context, module, builder, builder.CreateGlobalStringPtr(" -> "));
	std_print_i32(context, module, builder, builder.CreateLoad(varA, "a"));
	std_print(context, module, builder, builder.CreateGlobalStringPtr("\n"));
	builder.CreateStore(addA, varA);
	builder.CreateBr(incBB);

	builder.SetInsertPoint(incBB);
	varIv = builder.CreateLoad(varI, "i2");
	Value* incI = builder.CreateAdd(varIv, ConstantInt::get(context, APInt(32, 1)), "inc");
	builder.CreateStore(incI, varI);
	builder.CreateBr(condBB);

	builder.SetInsertPoint(exitBB);
	std_exit(context, module, builder, 0);

	return module;
}

Module* getStructModule (LLVMContext& context, int number, const char* message) {
	Module* module = new Module("struct", context);

	Type* Tvoid = Type::getVoidTy(context);
	Type* Ti32 = Type::getInt32Ty(context);
	Type* Ti8 = Type::getInt8Ty(context);

	vector<Type*> params;
	Function* mainFunction = makeFunction(module, "main", Tvoid, params);
	BasicBlock* block = BasicBlock::Create(context, "entry", mainFunction);
	IRBuilder<> builder(block);

	StructType::create("struct._a", Ti32, Ti8->getPointerTo());
	Type* _a = module->getTypeByName("struct._a");
	AllocaInst* varI32 = builder.CreateAlloca(_a, nullptr, "qwe");
	Value* structV1 = builder.CreateStructGEP(_a, varI32, 0);
	Value* structV2 = builder.CreateStructGEP(_a, varI32, 1);

	builder.CreateStore(ConstantInt::get(context, APInt(32, number)), structV1);
	builder.CreateStore(builder.CreateGlobalStringPtr(message), structV2);

	Value* newLine = builder.CreateGlobalStringPtr("\n");
	Value* label = builder.CreateGlobalStringPtr("[struct] ");
	std_print(context, module, builder, label);
	std_print_i32(context, module, builder, builder.CreateLoad(structV1, "tmp"));
	std_print(context, module, builder, newLine);
	std_print(context, module, builder, label);
	std_print(context, module, builder, builder.CreateLoad(structV2, "tmp"));

	std_exit(context, module, builder, 0);

	verifyModule(*module);
	return module;
}

int main (int argc, char** argv) {
	cl::SetVersionPrinter(printVersion);
	cl::HideUnrelatedOptions(LightCategory);
	cl::ParseCommandLineOptions(argc, argv);

	LLVMContext GlobalContext;
	LLVMBackend* backend = new LLVMBackend();

	for (auto &filename : InputFilenames) {
		Parser* parser = new Parser(filename.c_str());
		ASTStatements* stms = parser->program();
		stms->print(0);
		backend->writeObj(stms);
	}

	outs() << "\n--- OBJ ---\n";
	Module* module = getIfElseModule(GlobalContext, 12000);
	//module->print(outs(), nullptr);
	auto moduleName = "test\\" + module->getModuleIdentifier() + ".obj";
	backend->writeObj(module, moduleName.c_str());

	module = getForModule(GlobalContext, 5);
	//module->print(outs(), nullptr);
	moduleName = "test\\" + module->getModuleIdentifier() + ".obj";
	backend->writeObj(module, moduleName.c_str());

	module = getStructModule(GlobalContext, -42, "Ima structooo\n");
	//module->print(outs(), nullptr);
	moduleName = "test\\" + module->getModuleIdentifier() + ".obj";
	backend->writeObj(module, moduleName.c_str());

	delete module;

	return 0;
}
