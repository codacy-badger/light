#pragma once

#include "parser/parser.hpp"

struct Light_Compiler {
	Parser* parser = NULL;

	Light_Compiler ();

	static void report_info (Ast* node, char* format, ...);
	static void report_warning (Ast* node, char* format, ...);
	static void report_error (Ast* node, char* format, ...);

	Ast_Type_Instance* type_def_void;
	Ast_Type_Instance* type_def_i1;
	Ast_Type_Instance* type_def_i32;
};
