#include <stdlib.h>
#include <string.h>

#include "bytecode/interpreter.hpp"
#include "back/coff/coff_object.hpp"
#include "compiler.hpp"

#define NAME "Light Compiler"
#define VERSION "0.1.0"

int main (int argc, char** argv) {
	Light_Compiler* compiler = new Light_Compiler();

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-o") == 0) {
			if (compiler->settings->output_file) {
				compiler->error_stop(NULL, "Output file can only be set once");
			} else {
				compiler->settings->output_file = argv[++i];
			}
		} else {
			compiler->settings->input_files.push_back(argv[i]);
		}
	}

	compiler->run();
	delete compiler;

	return 0;
}
