#pragma once

#include "parser/ast.hpp"

struct Light_Compiler {
	Light_Compiler () {}
	~Light_Compiler () {}

	void report_info(AST* node, char* format, ...);
	void report_warning(AST* node, char* format, ...);
	void report_error(AST* node, char* format, ...);

	Ast_Type_Definition* type_def_void;
	Ast_Type_Definition* type_def_i1;
	Ast_Type_Definition* type_def_i8;
	Ast_Type_Definition* type_def_i16;
	Ast_Type_Definition* type_def_i32;
	Ast_Type_Definition* type_def_i64;
	Ast_Type_Definition* type_def_i128;
	Ast_Type_Definition* type_def_u8;
	Ast_Type_Definition* type_def_u16;
	Ast_Type_Definition* type_def_u32;
	Ast_Type_Definition* type_def_u64;
	Ast_Type_Definition* type_def_u128;
};
