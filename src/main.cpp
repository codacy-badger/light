#include <stdlib.h>
#include <string.h>

#include "bytecode/interpreter.hpp"
#include "back/coff/coff_object.hpp"
#include "compiler.hpp"
#include "report.hpp"

Compiler* g_compiler = NULL;

#define CHECK_ARG(arg) (strcmp(argv[i], arg) == 0)
#define CHECK_ARG_2(arg_short, arg_long) (CHECK_ARG(arg_short) || CHECK_ARG(arg_long))

void handle_compiler_arguments (Compiler_Settings* settings, int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (CHECK_ARG_2("-o", "-output")) {
				settings->output_file = argv[++i];
			} else if (CHECK_ARG_2("-v", "-verbose")) {
				settings->is_verbose = true;
			} else if (CHECK_ARG_2("-d", "-debug")) {
				settings->is_debug = true;
			}
		} else settings->input_files.push_back(argv[i]);
	}
}

int main (int argc, char** argv) {
	g_compiler = new Compiler();

	handle_compiler_arguments(g_compiler->settings, argc, argv);
	g_compiler->run();

	delete g_compiler;

	return 0;
}
