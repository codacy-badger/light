#pragma once

#include "build_settings.hpp"
#include "utils/event_queue.hpp"
#include "workspace.hpp"

#include <vector>
#include <assert.h>

#define COMPILER_NAME "Light Compiler"
#define COMPILER_VERSION "0.0.0"

struct Compiler {
	static Event_Queue event_queue;

	static size_t workspaces_created;
	static size_t workspaces_finished;

	static Workspace* create_workspace () {
		return new Workspace(Compiler::workspaces_created++);
	}

	static void apply_settings (Workspace* w, Build_Settings* settings) {
		w->settings = (*settings);
	}

	static void begin_build (Workspace* w) {
        printf("Starting workspace #%zd\n", w->guid);
		w->start_building(&Compiler::event_queue);
	}

	static Compiler_Event* get_next_event () {
		if (!Compiler::event_queue.empty()) {
			return Compiler::event_queue.pop();
		} else return NULL;
	}

	static void wait_for_end () {
		while (true) {
	        auto event = Compiler::get_next_event();
			if (!event) continue;

			assert(event->workspace != NULL);

	        if (event->kind == EVENT_COMPLETE) {
				Compiler::workspaces_finished++;
				Compiler::end_build(event->workspace);

				if (Compiler::workspaces_created == Compiler::workspaces_finished) {
					break;
				}
			} else if (event->kind == EVENT_FILE) {
				auto file_event = static_cast<Compiler_Event_File*>(event);
				switch (file_event->file_kind) {
					case FILE_OPEN:  { printf(" >> OPEN  "); break; }
					case FILE_CLOSE: { printf(" << CLOSE "); break; }
				}
				printf(file_event->absolute_path);
				printf("\n");
			}
	    }
	}

	static void end_build (Workspace* w) {
		printf("Workspace #%zd complete (%zd / %zd)\n", w->guid,
			Compiler::workspaces_finished, Compiler::workspaces_created);
		w->stop();
	}
};

Event_Queue Compiler::event_queue;
size_t Compiler::workspaces_created = 0;
size_t Compiler::workspaces_finished = 0;
