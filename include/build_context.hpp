#pragma once

#include "platform.hpp"
#include "os.hpp"
#include "arch.hpp"
#include "compiler_events.hpp"
#include "utils/async_queue.hpp"
#include "utils/string_map.hpp"
#include "ast/nodes.hpp"
#include "front/parser/parser.hpp"

// DEBUG INCLUDE
#include "ast/printer.hpp"

#include <vector>

struct Workspace;

struct Build_Context {
	const char* output = NULL;
	std::vector<const char*> input_files;

	char base_path[MAX_PATH_LENGTH];

	bool is_multithread = false;
	bool is_debug = false;

	OS* target_os = OS::get_current_os();
	Arch* target_arch = Arch::get_current_arch();

	// internal attributes for pipes

    Async_Queue<Compiler_Event> events;

    Workspace* workspace = NULL;
	Parser* parser = NULL;

	Ast_Printer* printer = new Ast_Printer();

	void init (Workspace* w) {
        this->parser = new Parser();
		os_get_current_directory(this->base_path);
		this->workspace = w;
	}

    void trigger (Compiler_Event event) {
        this->events.push(event);
    }
};
