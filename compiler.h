#include "parser/ast.h"

struct Light_Compiler {
	Light_Compiler () {}
	~Light_Compiler () {}

	void report_info(AST* node, char* format, ...);
	void report_warning(AST* node, char* format, ...);
	void report_error(AST* node, char* format, ...);

	ASTType* type_def_void;
	ASTType* type_def_i1;
	ASTType* type_def_i8;
	ASTType* type_def_i16;
	ASTType* type_def_i32;
	ASTType* type_def_i64;
	ASTType* type_def_i128;
};
