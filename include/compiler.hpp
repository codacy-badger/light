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
	static std::vector<Workspace*> workspaces;

	static Workspace* create_workspace (const char* name) {
		return new Workspace(name);
	}

	static void apply_settings (Workspace* w, Build_Settings* settings) {
		w->settings = (*settings);
	}

	static void begin_build (Workspace* w) {
        printf("Starting workspace '%s'...\n", w->name);
		Compiler::workspaces.push_back(w);
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
				auto v = &Compiler::workspaces;
				auto w = event->workspace;

				v->erase(std::remove(v->begin(), v->end(), w), v->end());
				Compiler::end_build(w);

				if (v->empty()) break;
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
		printf("Workspace '%s' complete\n", w->name);
		w->stop();
	}
};

Event_Queue Compiler::event_queue;
std::vector<Workspace*> Compiler::workspaces;
