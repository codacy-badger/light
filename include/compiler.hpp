#pragma once

#include "build_settings.hpp"
#include "compiler_events.hpp"
#include "workspace.hpp"

#define COMPILER_NAME "Light Compiler"
#define COMPILER_VERSION "0.0.0"

struct Compiler {
	static Workspace* create_workspace () {
		return new Workspace();
	}

	static void apply_settings (Workspace* w, Build_Settings* settings) {
		w->settings = (*settings);
	}

	static void begin_build (Workspace*) { /* empty */ }

	static Compiler_Event* wait_for_message (Workspace*) {
		auto event = new Compiler_Event();
		event->kind = EVENT_COMPLETE;
		return event;
	}

	static void wait_for_end (Workspace* w) {
		while (true) {
	        auto event = Compiler::wait_for_message(w);
	        if (event->kind == EVENT_COMPLETE) break;
	    }
	}
};
