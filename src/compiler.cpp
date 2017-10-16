#pragma once

#include "compiler.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

Light_Compiler::Light_Compiler () {
	this->parser = new Parser("");
}

void print_node_location (FILE* buffer, Ast* node) {
	assert(node);
	fprintf(buffer, "%s:%d,%d", node->filename, node->line, node->col);
}

void Light_Compiler::report_info (Ast* node, char* format, ...) {
	fprintf(stderr, "[INFO] ");

	va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
	fprintf(stdout, "\n\t");

	print_node_location(stdout, node);
}

void Light_Compiler::report_warning (Ast* node, char* format, ...) {
	fprintf(stderr, "[WARNING] ");

	va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
	fprintf(stdout, "\n\t");

	print_node_location(stdout, node);
}

void Light_Compiler::report_error (Ast* node, char* format, ...) {
	fprintf(stderr, "[ERROR] ");

	va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
	fprintf(stderr, "\n\t");

	print_node_location(stderr, node);
}

Ast_Type_Instance* type_def_void = new Ast_Struct_Type("void");
Ast_Type_Instance* type_def_i1 = new Ast_Struct_Type("i1");
Ast_Type_Instance* type_def_i32 = new Ast_Struct_Type("i32");
