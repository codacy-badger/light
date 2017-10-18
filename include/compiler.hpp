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

	Light_Compiler (Light_Compiler_Settings* settings = NULL);

	void run ();

	static void report_info (Ast* node, const char* format, ...);
	static void report_warning (Ast* node, const char* format, ...);
	static void report_error (Ast* node, const char* format, ...);

	static void report_info (Lexer* lexer, const char* format, ...);
	static void report_warning (Lexer* lexer, const char* format, ...);
	static void report_error (Lexer* lexer, const char* format, ...);

	static Ast_Type_Instance* type_def_void;
	static Ast_Type_Instance* type_def_i1;
	static Ast_Type_Instance* type_def_i32;
};
