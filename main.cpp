#include <iomanip>

#include "llvm/Support/CommandLine.h"
#include "timer.cpp"
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

		clock_t start, total;
	    start = total = clock();

	    parser->scope();
		Timer::print("  Parser ", start);
		parser->onFinish();
		Timer::print("TOTAL ", total);
	}

	return 0;
}
