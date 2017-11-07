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
	/*Bytecode_Interpreter* interpreter = new Bytecode_Interpreter();

	interpreter->run(new Inst_Copy_Const(2, (uint16_t)0xF0AF));
	interpreter->run(new Inst_Copy_Reg(1, 2));
	interpreter->run(new Inst_Stack_Alloca(0, 4));
	interpreter->run(new Inst_Add_I32(2, 1));
	interpreter->run(new Inst_Copy_Reg(3, 2));
	interpreter->run(new Inst_Add_Reg(2, 3));

	interpreter->dump();
	delete interpreter;*/

	Light_Compiler* compiler = new Light_Compiler();

	compiler->settings->input_files.push_back("examples\\simple_math.li");
	compiler->settings->output_file = "bin\\simple_math.obj";

	compiler->run();
	delete compiler;

	return 0;
}
