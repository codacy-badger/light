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
	Byte_Buffer* buffer = new Byte_Buffer();

	buffer->append_bytes(3, BYTECODE_STACK_ALLOCA, 1, 3);
	// a : i8 = 42;
	buffer->append_bytes(4, BYTECODE_STACK_OFFSET, 1, 0, 3);
	buffer->append_bytes(4, BYTECODE_STORE_INT, 1, 0, 42);
	// b : i8 = a + 2;
	buffer->append_bytes(4, BYTECODE_STACK_OFFSET, 1, 0, 3);
	buffer->append_bytes(4, BYTECODE_LOAD, 1, 1, 0);
	buffer->append_bytes(4, BYTECODE_ADD_INT, 1, 1, 2);
	buffer->append_bytes(4, BYTECODE_STACK_OFFSET, 1, 7, 2);
	buffer->append_bytes(4, BYTECODE_STORE, 1, 7, 1);
	// c : i8 = a + b + 7;
	buffer->append_bytes(4, BYTECODE_STACK_OFFSET, 1, 0, 3);
	buffer->append_bytes(4, BYTECODE_LOAD, 1, 1, 0);
	buffer->append_bytes(4, BYTECODE_STACK_OFFSET, 1, 0, 2);
	buffer->append_bytes(4, BYTECODE_LOAD, 1, 2, 0);
	buffer->append_bytes(4, BYTECODE_ADD, 1, 1, 2);
	buffer->append_bytes(4, BYTECODE_ADD_INT, 1, 1, 7);
	buffer->append_bytes(4, BYTECODE_STACK_OFFSET, 1, 7, 1);
	buffer->append_bytes(4, BYTECODE_STORE, 1, 7, 1);
	// END
	buffer->append_u8(BYTECODE_STOP);

	interpreter->run(buffer->data);
	interpreter->dump();

	delete buffer;
	delete interpreter;

	Light_Compiler* compiler = new Light_Compiler();

	compiler->settings->input_files.push_back("examples\\simple_math.li");
	compiler->settings->output_file = "bin\\simple_math.obj";

	compiler->run();

	return EXIT_SUCCESS;
}
