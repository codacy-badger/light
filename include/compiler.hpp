#pragma once

#include "compiler_settings.hpp"
#include "phase/compiler_phases.hpp"

#include "platform.hpp"

#include "modules.hpp"

#include "bytecode/interpreter.hpp"
#include "ast/types.hpp"

#include <chrono>

#define LIGHT_NAME "Light Compiler"
#define LIGHT_VERSION "0.1.0"

struct Compiler {
	Compiler_Settings* settings = new Compiler_Settings();
	Compiler_Phases* phases = NULL;

	Interpreter* interp = new Interpreter();
	Types* types = new Types();

	static Compiler* inst;

	std::chrono::high_resolution_clock::time_point clock_start;

	Compiler (int argc = 0, char** argv = NULL) {
		if (argc > 0) this->settings->handle_arguments(argc, argv);

		this->phases = new Compiler_Phases(this->settings);

		Compiler::inst = this;

		Events::add_observer(CE_COMPILER_ERROR, &Compiler::on_compiler_error, this);
		Events::add_observer(CE_COMPILER_STOP, &Compiler::on_compiler_stop, this);
	}

	void on_compiler_stop (void* data) {
		auto exit_code = reinterpret_cast<size_t>(data);

		this->phases->shutdown();

		auto stop = std::chrono::high_resolution_clock::now();
		auto interval_in_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(stop - this->clock_start);

		this->phases->print_metrics();

	    printf("\nCompleted in %8.6fs\n", interval_in_seconds.count());

		exit((int) exit_code);
	}

	void compile_input_files () {
		Events::trigger(CE_COMPILER_START, this);
		printf("%s v%s\n\n", LIGHT_NAME, LIGHT_VERSION);

		this->clock_start = std::chrono::high_resolution_clock::now();

		for (auto &filename : this->settings->input_files) {
			auto absolute_path = (char*) malloc(MAX_PATH_LENGTH);
		    os_get_absolute_path(filename, absolute_path);
			filename = absolute_path;

			Events::trigger(CE_IMPORT_MODULE, absolute_path);
		}

		this->phases->join();
		Events::trigger(CE_COMPILER_STOP);
	}

	void on_compiler_error (void* data) {
		auto error_description = reinterpret_cast<char*>(data);

		printf("Compiler error: %s\n", error_description);

		Events::trigger(CE_COMPILER_STOP, 1);
	}
};
