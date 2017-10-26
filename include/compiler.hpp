#pragma once

#include "parser/parser.hpp"

#include <vector>

using namespace std;

struct Light_Compiler_Settings {
	vector<const char*> input_files;
	const char* output_file;
};

struct Light_Compiler {
	Light_Compiler_Settings* settings = NULL;
	Parser* parser = NULL;

	static Ast_Type_Definition* type_def_void;
	static Ast_Type_Definition* type_def_i32;

	static Light_Compiler* instance;

	Light_Compiler (Light_Compiler_Settings* settings = NULL);

	void run ();

	void report_info (Ast* node, const char* format, ...);
	void report_warning (Ast* node, const char* format, ...);
	void report_error (Ast* node, const char* format, ...);
};
