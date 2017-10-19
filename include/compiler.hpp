#pragma once

#include "parser/parser.hpp"

#include <vector>
#include <set>

using namespace std;

struct Light_Compiler_Settings {
	vector<const char*> input_files;
	const char* output_file;
};

struct Light_Compiler {
	Light_Compiler_Settings* settings = NULL;
	Parser* parser = NULL;

	set<const char*> native_dependencies;

	Light_Compiler (Light_Compiler_Settings* settings = NULL);

	void run ();

	void report_info (Ast* node, const char* format, ...);
	void report_warning (Ast* node, const char* format, ...);
	void report_error (Ast* node, const char* format, ...);

	static Ast_Type_Instance* type_def_void;
	static Ast_Type_Instance* type_def_i1;
	static Ast_Type_Instance* type_def_i32;
};

extern Light_Compiler* global_compiler;
