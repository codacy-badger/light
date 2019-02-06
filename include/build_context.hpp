#pragma once

#include "platform.hpp"
#include "os.hpp"
#include "arch.hpp"
#include "compiler_events.hpp"
#include "utils/async_queue.hpp"

#include <vector>

struct Workspace;

struct Build_Context {
	const char* output = NULL;
	std::vector<const char*> input_files;

	char initial_path[MAX_PATH_LENGTH];

	bool is_multithread = false;
	bool is_debug = false;

	OS* target_os = OS::get_current_os();
	Arch* target_arch = Arch::get_current_arch();

    Async_Queue<Compiler_Event> events;

    Workspace* workspace = NULL;

	Build_Context () { os_get_current_directory(this->initial_path); }

    void trigger (Compiler_Event event) {
        this->events.push(event);
    }

    const char* find_absolute_path (const char* relative_path) {
		auto buffer = new char[MAX_PATH_LENGTH];
        os_get_absolute_path(relative_path, buffer);
		return buffer;
    }
};
