#include <stdlib.h>
#include <string.h>
#include <ctime>

#include "byte_buffer.hpp"
#include "bytecode/interpreter.hpp"
#include "back/coff/coff_object.hpp"
#include "compiler.hpp"
#include "timer.hpp"

using namespace std;

#define NAME "Light Compiler"
#define VERSION "0.1.0"

int main (int argc, char** argv) {
	Bytecode_Interpreter* interpreter = new Bytecode_Interpreter();

	interpreter->run(new Inst_Copy_Const(2, (uint16_t)0xF0AF));
	interpreter->run(new Inst_Copy_Reg(1, 2));
	interpreter->run(new Inst_Stack_Alloca(0, 4));

	interpreter->dump();
	delete interpreter;

	Light_Compiler* compiler = new Light_Compiler();

	compiler->settings->input_files.push_back("examples\\simple_math.li");
	compiler->settings->output_file = "bin\\simple_math.obj";

	compiler->run();

	return EXIT_SUCCESS;
}
