#include <stdlib.h>
#include <string.h>

#include "bytecode/interpreter.hpp"
#include "back/coff/coff_object.hpp"
#include "compiler.hpp"
#include "timer.hpp"

using namespace std;

#define NAME "Light Compiler"
#define VERSION "0.1.0"

int main (int argc, char** argv) {
	Light_Compiler* compiler = new Light_Compiler();

	compiler->settings->input_files.push_back("examples/simple_math.li");
	compiler->settings->output_file = "bin/simple_math.obj";

	compiler->run();
	delete compiler;

	return 0;
}
