#pragma once

#include "modules.hpp"
#include "code_source.hpp"
#include "pipeline/pipeline.hpp"

#include "bytecode/interpreter.hpp"
#include "ast/parser.hpp"
#include "ast/types.hpp"

#include <vector>
#include <queue>

using namespace std;

struct Compiler_Settings {
	vector<const char*> input_files;
	const char* output = NULL;

	char initial_path[MAX_PATH_LENGTH];

	bool is_verbose = false;
	bool is_debug = false;

	uint8_t register_size = 8;

	Compiler_Settings(int argc, char** argv);

    void handle_arguments (int argc, char** argv);
};

struct Compiler {
	Compiler_Settings* settings;

	queue<Code_Source*> code_sources;

	Modules* modules = new Modules();

	Interpreter* interp = new Interpreter();
	Types* types = new Types();

	Compiler (Compiler_Settings* settings);

	void run ();
	void quit ();

	/* static methods */

	static Compiler* instance;

	static Compiler* create(int argc, char** argv);
	static Compiler* create(Compiler_Settings* settings = NULL);
	static void destroy();
};
