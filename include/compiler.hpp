#pragma once

#include "compiler_settings.hpp"

#include "platform.hpp"

#include "modules.hpp"
#include "code_input.hpp"
#include "pipeline/pipeline.hpp"

#include "bytecode/interpreter.hpp"
#include "ast/parser.hpp"
#include "ast/types.hpp"

#include <queue>

using namespace std;

#define LIGHT_NAME "Light Compiler"
#define LIGHT_VERSION "0.1.0"

#define COMPILER_DONE_FORMAT "\nCompleted in %8.6fs (%8.6fs)\n"

struct Compiler {
	Compiler_Settings* settings = new Compiler_Settings();

	queue<const char*> code_sources;

	Interpreter* interp = new Interpreter();
	Types* types = new Types();
	Modules* modules = new Modules(this);

	static Compiler* inst;

	Compiler (int argc = 0, char** argv = NULL) {
		if (argc > 0) {
			this->settings->handle_arguments(argc, argv);
		}
		Compiler::inst = this;

		Events::add_observer(CE_COMPILER_ERROR, &Compiler::handle_compiler_error, this);
		Events::add_observer(CE_COMPILER_STOP, &Compiler::handle_compiler_stop, this);

		Events::trigger(CE_COMPILER_START, this);
	}

	void handle_compiler_stop (void* data) {
		auto exit_code = reinterpret_cast<size_t>(data);

		if (exit_code != 0) exit((int) exit_code);
	}

	void handle_compiler_error (void* data) {
		auto error_description = reinterpret_cast<char*>(data);

		printf("Compiler error: %s\n", error_description);

		Events::trigger(CE_COMPILER_STOP, 1);
	}

	void compile_input_files () {
		printf("%s v%s\n\n", LIGHT_NAME, LIGHT_VERSION);

	    auto totalWall = os_get_wall_time();
	    auto totalUser = os_get_user_time();

		for (auto filename : this->settings->input_files) {
			auto absolute_path = (char*) malloc(MAX_PATH_LENGTH);
		    os_get_absolute_path(filename, absolute_path);

			Events::trigger(CE_IMPORT_MODULE, absolute_path);
		}

		auto userInterval = os_time_user_stop(totalUser);
		auto wallInterval = os_time_wall_stop(totalWall);

		this->modules->print_metrics(userInterval);

	    printf(COMPILER_DONE_FORMAT, userInterval, wallInterval);
	}
};
