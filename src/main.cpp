#include <iomanip>
#include <stdlib.h>

#include "llvm/Support/CommandLine.h"

#include "compiler.hpp"
#include "timer.hpp"
#include "parser/parser.hpp"
#include "parser/printer.hpp"

#include "parser/pipe/print_pipe.hpp"
#include "back/llvm/llvm_pipe.hpp"

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

void link (std::string output) {
	auto linker = clock();
	std::string linkerCommand = "link /nologo /ENTRY:main ";

	linkerCommand += "/OUT:\"" + output + "\" ";

	linkerCommand += "_tmp_.obj ";
	linkerCommand += "build\\std_li.lib";

	system(linkerCommand.c_str());
	system("del _tmp_.obj");
	Timer::print("  Link  ", linker);
}

int main (int argc, char** argv) {
	cl::SetVersionPrinter(printVersion);
	cl::HideUnrelatedOptions(LightCategory);
	cl::ParseCommandLineOptions(argc, argv);

	std::cout << std::fixed << std::setprecision(3);

	auto total = clock();
	for (auto &filename : InputFilenames) {
		cout << filename.c_str() << "\n";

		auto parser = new Parser(filename.c_str());
		parser->append(new PrintPipe());
		//parser->append(new LLVMPipe("_tmp_.obj"));

		clock_t start = clock();
		parser->block();
		Timer::print("  Parse ", start);
		parser->onFinish();
		//link(OutputFilename.c_str());
	}
	Timer::print("TOTAL   ", total);

	return 0;
}
