#include <stdlib.h>
#include <string.h>
#include <ctime>

#include "llvm/Support/CommandLine.h"

#include "byte_buffer.hpp"
#include "bytecode/interpreter.hpp"
#include "back/coff/coff_object.hpp"
#include "compiler.hpp"
#include "timer.hpp"

using namespace llvm;
using namespace std;

#define NAME "Light Compiler"
#define VERSION "0.1.0"

cl::OptionCategory LightCategory("Compiler Options");

static cl::opt<string>
OutputFilename("o", cl::desc("Specify output file."), cl::value_desc("filename"), cl::cat(LightCategory));

static cl::list<string>
InputFilenames(cl::Positional, cl::desc("<input files>"), cl::OneOrMore);

void printVersion (raw_ostream& out) {
	printf("%s %s\n", NAME, VERSION);
}

int main (int argc, char** argv) {

	Bytecode_Interpreter* interp = new Bytecode_Interpreter();

	Byte_Buffer* buffer = new Byte_Buffer();
	buffer->append_bytes(3, BYTECODE_ALLOCA, 2, 0);
	buffer->append_bytes(3, BYTECODE_STOREI, 2, 0);
	buffer->append_2b(62121);
	buffer->append_1b(BYTECODE_STOP);
	interp->instructions = buffer->buffer;

	interp->start();
	interp->dump();

	delete buffer;
	delete interp;

	/*cl::SetVersionPrinter(printVersion);
	cl::HideUnrelatedOptions(LightCategory);
	cl::ParseCommandLineOptions(argc, argv);

	global_compiler = new Light_Compiler();

	for (auto filename : InputFilenames) {
		char* _source = (char*) malloc(filename.size() * sizeof(char));
		strcpy(_source, filename.c_str());
		global_compiler->settings->input_files.push_back(_source);
	}

	char* _output = (char*) malloc(OutputFilename.size() * sizeof(char));
	strcpy(_output, OutputFilename.c_str());
	global_compiler->settings->output_file = _output;

	global_compiler->run();*/

	return EXIT_SUCCESS;
}
