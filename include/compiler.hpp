#pragma once

#include "bytecode/interpreter.hpp"
#include "parser/parser.hpp"
#include "types.hpp"

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
	Bytecode_Interpreter* interp = new Bytecode_Interpreter();
	Types* types = new Types();
	Parser* parser = NULL;

	Byte_Order byte_order = BYTEORDER_UNDEFINED;
	bool has_errors = false;

	Ast_Struct_Type* type_def_type;
	Ast_Struct_Type* type_def_void;
	Ast_Struct_Type* type_def_bool;
	Ast_Struct_Type* type_def_s8;
	Ast_Struct_Type* type_def_s16;
	Ast_Struct_Type* type_def_s32;
	Ast_Struct_Type* type_def_s64;
	Ast_Struct_Type* type_def_u8;
	Ast_Struct_Type* type_def_u16;
	Ast_Struct_Type* type_def_u32;
	Ast_Struct_Type* type_def_u64;
	Ast_Struct_Type* type_def_f32;
	Ast_Struct_Type* type_def_f64;
	Ast_Pointer_Type* type_def_string;

	static Light_Compiler* inst;

	Light_Compiler (Light_Compiler_Settings* settings = NULL);

	void run ();

	void info (Ast* node, const char* format, ...);
	void warning (Ast* node, const char* format, ...);
	void error (Ast* node, const char* format, ...);
	void v_error (Ast* node, const char* format, va_list argptr);
	void error_stop (Ast* node, const char* format, ...);

	void stop ();
};
