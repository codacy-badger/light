#include "llvm/Support/CommandLine.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include <iomanip>

#include "parser/parser.cpp"
#include "parser/printer.cpp"
#include "back/llvm/llvm_pipe.cpp"

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

double clockStop (clock_t start) {
	return (clock() - start) / static_cast<double>(CLOCKS_PER_SEC);
}

int main (int argc, char** argv) {
	cl::SetVersionPrinter(printVersion);
	cl::HideUnrelatedOptions(LightCategory);
	cl::ParseCommandLineOptions(argc, argv);

	std::cout << std::fixed << std::setprecision(3);

	for (auto &filename : InputFilenames) {
		auto parser = new Parser(filename.c_str());
		parser->append(new LLVMPipe(OutputFilename.c_str()));

		clock_t start, front, total;
	    start = front = total = clock();
	    auto globalScope = parser->program();
		std::cout << "  AST Parser " << clockStop(start) << "s" << std::endl;

		if (globalScope) {
			std::cout << "Frontend " << clockStop(front) << "s" << std::endl;
			ASTPrinter::print(globalScope);
		    start = clock();
			std::cout << "LLVM Backend " << clockStop(start) << "s" << std::endl;
			std::cout << "TOTAL " << clockStop(total) << "s" << std::endl;
		}
	}

	return 0;
}
