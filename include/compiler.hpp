#pragma once

#include "compiler_settings.hpp"
#include "phase/compiler_phases.hpp"

#include "bytecode/interpreter.hpp"
#include "ast/types.hpp"

#include "util/timer.hpp"

#define LIGHT_NAME "Light Compiler"
#define LIGHT_VERSION "0.0.0"

struct Compiler {
	Compiler_Settings settings;
	Compiler_Phases* phases;

	Timer user_timer = Timer(TIMER_TYPE_USER_TIME);
	Timer wall_timer;

	Interpreter* interp = new Interpreter();
	Types* types = new Types();

	// @TODO @FIXME this property should not exists, it makes everything
	// harder to reason about and complicates debugging
	static Compiler* inst;

	Event_Queue event_queue;

	Compiler (int argc = 0, char** argv = NULL) {
		this->settings.handle_arguments(argc, argv);
		this->phases = new Compiler_Phases(&this->settings);

		Compiler::inst = this;

		Events::bind(CE_COMPILER_ERROR, &this->event_queue);
	}

	void compile_input_files () {
		printf("%s v%s\n\n", LIGHT_NAME, LIGHT_VERSION);

		this->wall_timer.start();
		this->user_timer.start();

		for (auto &absolute_path : this->settings.input_files) {
			Events::trigger(CE_IMPORT_MODULE, absolute_path);
		}

		while (!this->is_all_work_done()) {
			this->handle_compiler_events();
			if (this->settings.is_multithread) {
				std::this_thread::sleep_for(100ns);
			}
		}

		this->phases->shutdown();

		auto executing_time = this->user_timer.stop();
		auto running_time = this->wall_timer.stop();
		this->phases->print_metrics();
	    printf("\nDone in %8.6fs (%8.6fs)\n", running_time, executing_time);
	}

	void handle_compiler_events () {
		while (!this->event_queue.empty()) {
			auto event = this->event_queue.pop();

			switch (event.id) {
				case CE_COMPILER_ERROR: {
					printf("Errors found during compilation...\n");
					exit(1);
				}
			}
		}
	}

	bool is_all_work_done () {
		return this->phases->are_all_done() && this->event_queue.empty();
	}
};
