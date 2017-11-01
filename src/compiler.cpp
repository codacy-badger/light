#pragma once

#include "compiler.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#include "timer.hpp"
#include "parser/pipe/symbol_resolution.hpp"
#include "parser/pipe/Type_Checking.hpp"
#include "parser/pipe/print_pipe.hpp"

Light_Compiler* Light_Compiler::instance = NULL;

void link (std::string output) {
	auto linker = Timer::getTime();
	std::string linkerCommand = "link /nologo /ENTRY:main ";

	linkerCommand += "/OUT:\"" + output + "\" ";

	linkerCommand += "_tmp_.obj ";
	linkerCommand += "build\\std_li.lib";

	system(linkerCommand.c_str());
	system("del _tmp_.obj");
	Timer::print("  Link  ", linker);
}

Light_Compiler::Light_Compiler (Light_Compiler_Settings* settings) {
	if (!settings) this->settings = new Light_Compiler_Settings();
	else this->settings = settings;
	assert(Light_Compiler::instance == NULL);
	Light_Compiler::instance = this;

	this->type_def_type = new Ast_Type_Definition();
	this->type_def_type->inferred_type = this->type_def_type;
	this->type_def_type->typedef_type = AST_TYPEDEF_TYPE;

	this->type_def_void = new Ast_Struct_Type();
	this->type_def_void->inferred_type = this->type_def_type;
	this->type_def_void->typedef_type = AST_TYPEDEF_STRUCT;
	this->type_def_void->name = "void";

	this->type_def_i32 = new Ast_Struct_Type();
	this->type_def_i32->inferred_type = this->type_def_type;
	this->type_def_i32->typedef_type = AST_TYPEDEF_STRUCT;
	this->type_def_i32->name = "i32";
}

void Light_Compiler::run () {
	auto total = Timer::getTime();
	for (auto filename : this->settings->input_files) {
		printf("%s\n", filename);

		auto parser = new Parser(this, filename);
		parser->append(new Symbol_Resolution());
		parser->append(new Type_Checking());
		parser->append(new PrintPipe());

		auto start = Timer::getTime();
		parser->top_level_block();
		Timer::print("  Parse ", start);
		parser->on_finish();
	}
	Timer::print("TOTAL   ", total);
}

void print_node_location (FILE* buffer, Ast* node) {
	if (node)
		fprintf(buffer, "@ %s:%d,%d", node->filename, node->line, node->col);
	else fprintf(buffer, "@ (null):?,?");
}

void Light_Compiler::report_info (Ast* node, const char* format, ...) {
	fprintf(stderr, "[INFO] ");

	va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
	fprintf(stdout, "\n\t");

	print_node_location(stdout, node);
	fprintf(stdout, "\n");
}

void Light_Compiler::report_warning (Ast* node, const char* format, ...) {
	fprintf(stderr, "[WARNING] ");

	va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
	fprintf(stdout, "\n\t");

	print_node_location(stdout, node);
	fprintf(stdout, "\n");
}

void Light_Compiler::report_error (Ast* node, const char* format, ...) {
	fprintf(stderr, "[ERROR] ");

	va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
	fprintf(stderr, "\n\t");

	print_node_location(stderr, node);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}
