#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "../parser/ast/ast_type.cpp"
#include "../parser/ast/expression/ast_expression.cpp"
#include "../parser/ast/statement/ast_statements.cpp"

#include <string>
#include <map>

// Linker command: link /ENTRY:mainCRTStartup /OUT:demo2.exe /SUBSYSTEM:WINDOWS output.o
class LLVMBackend {
public:
	llvm::LLVMContext TheContext;
	llvm::IRBuilder<>* Builder;
	std::unique_ptr<llvm::Module> TheModule;
	std::map<std::string, llvm::Value*> NamedValues;

	LLVMBackend () {
		Builder = new llvm::IRBuilder<>(TheContext);
	}

	void LogError (const char * message) {
		printf("%s", message);
	}

	~LLVMBackend () {
		delete Builder;
	}
};
