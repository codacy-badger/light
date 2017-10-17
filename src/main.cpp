#include <stdlib.h>
#include <string.h>

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

int main (int argc, char** argv) {
	cl::SetVersionPrinter(printVersion);
	cl::HideUnrelatedOptions(LightCategory);
	cl::ParseCommandLineOptions(argc, argv);

	Light_Compiler* compiler = new Light_Compiler();

	for (auto filename : InputFilenames) {
		char* _source = (char*) malloc(filename.size() * sizeof(char));
		strcpy(_source, filename.c_str());
		compiler->settings->input_files.push_back(_source);
	}

	char* _output = (char*) malloc(OutputFilename.size() * sizeof(char));
	strcpy(_output, OutputFilename.c_str());
	compiler->settings->output_file = _output;

	compiler->run();

	return EXIT_SUCCESS;
}
