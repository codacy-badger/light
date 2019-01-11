#pragma once

#include "compiler_settings.hpp"
#include "phase/compiler_phases.hpp"

#include "platform.hpp"

#include "modules.hpp"

#include "bytecode/interpreter.hpp"
#include "ast/types.hpp"

#include "util/timer.hpp"

#define LIGHT_NAME "Light Compiler"
#define LIGHT_VERSION "0.1.0"

struct Compiler {
	Compiler_Settings settings;

	Compiler_Phases* phases = NULL;
	Timer timer;

	Interpreter* interp = new Interpreter();
	Types* types = new Types();

	static Compiler* inst;

	std::chrono::high_resolution_clock::time_point clock_start;
	Event_Queue event_queue;

	Compiler (int argc = 0, char** argv = NULL) {
		if (argc > 0) this->settings.handle_arguments(argc, argv);

		this->phases = new Compiler_Phases(&this->settings);

		Compiler::inst = this;

		Events::bind(CE_COMPILER_ERROR, &this->event_queue);
	}

	void compile_input_files () {
		printf("%s v%s\n\n", LIGHT_NAME, LIGHT_VERSION);

		this->timer.start();

		for (auto &absolute_path : this->settings.input_files) {
			Events::trigger(CE_IMPORT_MODULE, absolute_path);
		}

		while (!this->is_all_work_done()) {
			this->handle_compiler_events();
			std::this_thread::sleep_for(100ns);
		}

		this->phases->shutdown();

		auto total_interval = this->timer.stop().count();
		this->phases->print_metrics();
	    printf("\nCompleted in %8.6fs\n", total_interval);
	}

	void handle_compiler_events () {
		while (!this->event_queue.empty()) {
			auto event = this->event_queue.pop();

			switch (event.id) {
				case CE_COMPILER_ERROR: this->on_compiler_error(event.data);
			}
		}
	}

	void on_compiler_error (void* data) {
		auto error_description = reinterpret_cast<char*>(data);

		printf("Compiler error: %s\n", error_description);

		exit(1);
	}

	bool is_all_work_done () {
		return this->phases->are_all_done() && this->event_queue.empty();
	}
};
