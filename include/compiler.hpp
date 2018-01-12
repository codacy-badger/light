#pragma once

#include "bytecode/interpreter.hpp"
#include "parser/parser.hpp"
#include "types.hpp"

#include <vector>

#define LIGHT_NAME "Light Compiler"
#define LIGHT_VERSION "0.1.0"

using namespace std;

struct Compiler_Settings {
	vector<const char*> input_files;
	const char* output_file = NULL;

	bool is_verbose = false;
	bool is_debug = false;
};

struct Compiler {
	Compiler_Settings* settings = new Compiler_Settings();

	Interpreter* interp = new Interpreter();
	Parser* parser = new Parser();
	Types* types = new Types();

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

	Ast_Pointer_Type* type_def_u8_ptr;

	Compiler ();

	void run ();
	void stop ();

	void add_import (Ast_Import* import);
};

extern Compiler* g_compiler;
