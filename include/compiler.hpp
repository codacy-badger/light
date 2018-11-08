#pragma once

#include "bytecode/interpreter.hpp"
#include "parser/parser.hpp"

#include <vector>

#define LIGHT_NAME "Light Compiler"
#define LIGHT_VERSION "0.1.0"

#define UKNOWN_ARG_FORMAT "Unkown compiler argument at %d: \"%s\" (Ignored)"

#define CHECK_ARG(arg) (strcmp(argv[i], arg) == 0)
#define CHECK_ARG_2(arg_short, arg_long) (CHECK_ARG(arg_short) || CHECK_ARG(arg_long))

using namespace std;

struct Compiler_Settings {
	vector<const char*> input_files;
	const char* output_file = NULL;

	char initial_path[MAX_PATH_LENGTH];

	bool is_verbose = false;
	bool is_debug = false;

	uint8_t register_size = 8;

    void handle_arguments (int argc, char** argv) {
        for (int i = 1; i < argc; i++) {
            if (argv[i][0] == '-') {
                if (CHECK_ARG_2("-o", "-output")) {
                    this->output_file = argv[++i];
                } else if (CHECK_ARG_2("-v", "-verbose")) {
                    this->is_verbose = true;
                } else if (CHECK_ARG_2("-d", "-debug")) {
                    this->is_debug = true;
                } else report_warning(NULL, UKNOWN_ARG_FORMAT, i, argv[i]);
            } else this->input_files.push_back(argv[i]);
        }
    }
};

struct Compiler {
	Compiler_Settings* settings = NULL;

	Interpreter* interp = new Interpreter();
	Parser* parser = new Parser();
	Types* types = new Types();

	Compiler (Compiler_Settings* settings = NULL);

	void run ();
	void quit ();

	void add_import (Ast_Import* import);

	/* static methods */

	static Compiler* instance;

	static Compiler* create(int argc, char** argv);
	static Compiler* create(Compiler_Settings* settings = NULL);
	static void destroy();
};
