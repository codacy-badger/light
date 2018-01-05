#pragma once

#include "bytecode/interpreter.hpp"
#include "parser/parser.hpp"
#include "types.hpp"

#include <vector>

using namespace std;

struct Compiler_Settings {
	vector<const char*> input_files;
	const char* output_file;
};

struct Compiler {
	Compiler_Settings* settings = new Compiler_Settings();

	Interpreter* interp = new Interpreter();
	Types* types = new Types();
	Parser* parser = NULL;

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
	Ast_Struct_Type* type_def_usize;

	Compiler ();

	void run ();
	void stop ();
};

extern Compiler* g_compiler;
