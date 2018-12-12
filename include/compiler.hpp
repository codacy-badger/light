#pragma once

#include "pipeline/pipeline.hpp"

#include "bytecode/interpreter.hpp"
#include "ast/parser.hpp"
#include "ast/types.hpp"

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

    void handle_arguments (int argc, char** argv);
};

struct Compiler {
	Compiler_Settings* settings = NULL;

	Pipeline* pipeline = new Pipeline();

	Interpreter* interp = new Interpreter();
	Types* types = new Types();

	Compiler (Compiler_Settings* settings = NULL);

	void run ();
	void quit ();

	/* static methods */

	static Compiler* instance;

	static Compiler* create(int argc, char** argv);
	static Compiler* create(Compiler_Settings* settings = NULL);
	static void destroy();
};
