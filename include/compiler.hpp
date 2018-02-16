#pragma once

#include "bytecode/interpreter.hpp"
#include "parser/parser.hpp"

#include <vector>

#define LIGHT_NAME "Light Compiler"
#define LIGHT_VERSION "0.1.0"

using namespace std;

struct Compiler_Settings {
	vector<const char*> input_files;
	const char* output_file = NULL;

	char initial_path[MAX_PATH_LENGTH];

	bool is_verbose = false;
	bool is_debug = false;

	uint8_t register_size = 8;
};

struct Compiler {
	Compiler_Settings* settings = new Compiler_Settings();

	Interpreter* interp = new Interpreter();
	Parser* parser = new Parser();
	Types* types = new Types();

	Compiler ();

	void run ();
	void stop ();

	void add_import (Ast_Import* import);
};

extern Compiler* g_compiler;
