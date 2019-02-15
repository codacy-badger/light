#pragma once

#include "platform.hpp"
#include "os.hpp"
#include "arch.hpp"
#include "utils/async_queue.hpp"
#include "utils/string_map.hpp"
#include "ast/nodes.hpp"

#include <vector>

struct Workspace;
struct Parser;
struct Type_Inferrer;
struct Type_Table;
struct Type_Caster;
struct Ast_Printer;

struct Build_Context {
	const char* output = NULL;
	std::vector<const char*> input_files;

	char base_path[MAX_PATH_LENGTH];

	bool is_multithread = false;
	bool is_debug = false;

	OS* target_os = OS::get_current_os();
	Arch* target_arch = Arch::get_current_arch();

	// internal attributes for pipes

    Workspace* workspace = NULL;
	Parser* parser = NULL;

	Type_Inferrer* type_inferrer = NULL;
	Type_Table* type_table = NULL;
	Type_Caster* type_caster = NULL;

	Ast_Printer* printer = NULL;

	void init (Workspace* w);

	void debug (const char* format, ...);
	void debug (Ast* node, const char* format, ...);
	void debug (Location* location, const char* format, ...);

	void error (const char* format, ...);
	void error (Ast* node, const char* format, ...);
	void error (Location* location, const char* format, ...);

	void print_location (Location* location);

	void debug_v (const char* format, va_list args);
	void error_v (const char* format, va_list args);
};
