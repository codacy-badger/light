#pragma once

#include "compiler_settings.hpp"
#include "phase/compiler_phases.hpp"

#include "util/timer.hpp"

#define LIGHT_NAME "Light Compiler"
#define LIGHT_VERSION "0.0.0"

struct Compiler {
	Compiler_Settings settings;
	Compiler_Phases* phases;

	Timer timer;

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
		this->timer.start();
		printf("%s v%s\n\n", LIGHT_NAME, LIGHT_VERSION);

		for (auto &absolute_path : this->settings.input_files) {
			Events::trigger(CE_IMPORT_MODULE, absolute_path);
		}

		while (!this->is_all_work_done()) {
			this->handle_compiler_events();
		}

		this->phases->shutdown();
		this->phases->print_metrics();
	    printf("\nDone in %8.6fs\n", this->timer.stop());
	}

	void handle_compiler_events () {
		while (!this->event_queue.empty()) {
			auto event = this->event_queue.pop();

			switch (event.id) {
				case CE_COMPILER_ERROR: {
					this->phases->shutdown();
					printf("\nErrors found during compilation...\n");
					exit(1);
					break;
				}
			}
		}
	}

	bool is_all_work_done () {
		return this->phases->are_all_done() && this->event_queue.empty();
	}
};
