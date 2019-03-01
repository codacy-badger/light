#pragma once

#include "platform.hpp"
#include "os.hpp"
#include "arch.hpp"
#include "ast/nodes.hpp"

#include "utils/array.hpp"
#include "utils/string_map.hpp"
#include "utils/memory_arena.hpp"

struct Workspace;
struct Parser;
struct Modules;
struct Type_Inferrer;
struct Type_Table;
struct Type_Caster;
struct Ast_Printer;

struct Build_Context {
	const char* output = NULL;
	Array<const char*> input_files;

	char base_path[MAX_PATH_LENGTH];

	bool is_multithread = false;
	bool is_debug = false;

	bool has_error = false;

	OS* target_os = OS::get_current_os();
	Arch* target_arch = Arch::get_current_arch();

	// internal attributes for pipes

    Workspace* workspace = NULL;

	Memory_Arena* arena = NULL;

	Parser* parser = NULL;

    Modules* modules = NULL;
	Type_Inferrer* type_inferrer = NULL;
	Type_Table* type_table = NULL;
	Type_Caster* type_caster = NULL;

	Ast_Printer* printer = NULL;

	void init (Workspace* w);

	void debug (const char* format, ...);
	void debug (Ast* node, const char* format, ...);

	void error (const char* format, ...);
	void error (Ast* node, const char* format, ...);

	void shutdown ();

	void print_location (const char* path, size_t line);

	void debug_v (const char* format, va_list args);
	void error_v (const char* format, va_list args);
};
