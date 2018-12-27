#pragma once

#include "os/os.hpp"
#include "arch/arch.hpp"

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

	OS* target_os = OS::get_current_os();
	Arch* target_arch = Arch::get_current_arch();

	uint8_t register_size = 8;

	Compiler_Settings (int argc, char** argv);

    void handle_arguments (int argc, char** argv);
};

struct Compiler {
	Compiler_Settings* settings;

	queue<Code_Input*> code_sources;

	Modules* modules;

	Interpreter* interp = new Interpreter();
	Types* types = new Types();

	Compiler (Compiler_Settings* settings);

	void run ();
	void quit ();

	/* static methods */

	static Compiler* inst;

	static Compiler* create(int argc, char** argv);
	static Compiler* create(Compiler_Settings* settings = NULL);
	static void destroy();
};
