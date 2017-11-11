#pragma once

#include "bytecode/interpreter.hpp"
#include "parser/parser.hpp"

#include <stdarg.h>
#include <vector>

using namespace std;

struct Light_Compiler_Settings {
	vector<const char*> input_files;
	const char* output_file;
};

enum Byte_Order {
	BYTEORDER_UNDEFINED = 0,
	BYTEORDER_BIG_ENDIAN,
	BYTEORDER_LITTLE_ENDIAN,
};

struct Light_Compiler {
	Light_Compiler_Settings* settings = NULL;
	Bytecode_Interpreter* interp = NULL;
	Parser* parser = NULL;

	Byte_Order byte_order = BYTEORDER_UNDEFINED;
	bool has_errors = false;

	Ast_Type_Definition* type_def_type;
	Ast_Struct_Type* type_def_void;
	Ast_Struct_Type* type_def_s32;

	static Light_Compiler* instance;

	Light_Compiler (Light_Compiler_Settings* settings = NULL);

	void run ();

	void info (Ast* node, const char* format, ...);
	void warning (Ast* node, const char* format, ...);
	void error (Ast* node, const char* format, ...);
	void v_error (Ast* node, const char* format, va_list argptr);
	void error_stop (Ast* node, const char* format, ...);

	void stop ();
};
