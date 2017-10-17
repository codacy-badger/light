#include <iomanip>
#include <stdlib.h>

#include "llvm/Support/CommandLine.h"

#include "compiler.hpp"
#include "timer.hpp"

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

	Light_Compiler* compiler = new Light_Compiler();
	for (auto filename : InputFilenames)
		compiler->settings->input_files.push_back(filename.c_str());
	compiler->settings->output_file = OutputFilename.c_str();
	compiler->run();

	return EXIT_SUCCESS;
}
