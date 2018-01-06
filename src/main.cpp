#include <stdlib.h>
#include <string.h>

#include "bytecode/interpreter.hpp"
#include "back/coff/coff_object.hpp"
#include "compiler.hpp"

Compiler* g_compiler = NULL;

int main (int argc, char** argv) {
	g_compiler = new Compiler();

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-o") == 0) {
			if (g_compiler->settings->output_file) {
				report_error_stop(NULL, "Output file can only be set once");
			} else {
				g_compiler->settings->output_file = argv[++i];
			}
		} else {
			g_compiler->settings->input_files.push_back(argv[i]);
		}
	}

	g_compiler->run();
	delete g_compiler;

	return 0;
}
