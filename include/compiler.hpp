#pragma once

#include "compiler_settings.hpp"
#include "phase/compiler_phases.hpp"

struct Compiler {
	Compiler_Settings settings;
	Compiler_Phases* phases;

	// @TODO @FIXME NO NO NO, bad programmer, don't be bad!
	static Compiler* inst;

	Event_Queue event_queue;

	Compiler (int argc = 0, char** argv = NULL) {
		this->settings.handle_arguments(argc, argv);

		Logger::debug(COMPILER_NAME " v" COMPILER_VERSION);
		this->phases = new Compiler_Phases(&this->settings);

		if (Logger::is_debug()) {
			size_t async_count = 0;
			for (auto phase : this->phases->phases) {
				if (phase->is_async) async_count++;
			}
			Logger::debug("Compiler phases setup: %zd (%zd async)",
				this->phases->phases.size(), async_count);
		}

		Compiler::inst = this;

		Events::bind(CE_COMPILER_ERROR, &this->event_queue);
	}

	void compile_input_files () {
		auto start = os_get_time();

		for (auto &absolute_path : this->settings.input_files) {
			Events::trigger(CE_IMPORT_MODULE, absolute_path);
		}

		while (!this->phases->all_done()) {
			this->handle_compiler_events();
			this->phases->handle_sync_events();
		}

		this->phases->shutdown();
		this->phases->print_metrics();
	    printf("Done in %8.6fs\n", os_time_stop(start));
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
};
