#pragma once

#include "compiler.hpp"

#include <assert.h>
#include <stdio.h>

#include "timer.hpp"
#include "parser/pipe/symbol_resolution.hpp"
#include "parser/pipe/constant_folding.hpp"
#include "parser/pipe/type_checking.hpp"
#include "parser/pipe/foreign_function.hpp"
#include "parser/pipe/print_pipe.hpp"

#include "parser/pipe/struct_sizer.hpp"
#include "bytecode/bytecode_generator.hpp"
#include "bytecode/bytecode_runner.hpp"

Light_Compiler* Light_Compiler::inst = NULL;

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
	assert(Light_Compiler::inst == NULL);
	Light_Compiler::inst = this;

	int16_t i = 1;
	if (((int8_t *) &i)[0] == 1) {
		this->byte_order = BYTEORDER_LITTLE_ENDIAN;
	} else this->byte_order = BYTEORDER_BIG_ENDIAN;

	this->type_def_type = new Ast_Type_Definition();
	this->type_def_type->inferred_type = this->type_def_type;
	this->type_def_type->typedef_type = AST_TYPEDEF_TYPE;

	this->type_def_void = new Ast_Struct_Type("void");
	this->type_def_void->byte_size = 0;
	this->type_def_s32 = new Ast_Struct_Type("s32");
	this->type_def_s32->byte_size = 4;
}

void Light_Compiler::run () {
	auto total = Timer::getTime();
	for (auto filename : this->settings->input_files) {
		printf("%s\n", filename);

		auto parser = new Parser(this, filename);
		parser->append(new Symbol_Resolution());
		parser->append(new Constant_Folding());
		parser->append(new Type_Checking());
		parser->append(new Foreign_Function());
		parser->append(new Struct_Sizer());
		parser->append(new Bytecode_Generator());
		parser->append(new Bytecode_Runner());
		parser->append(new PrintPipe());

		auto start = Timer::getTime();
		parser->top_level_block();
		Timer::print("\n  Parse ", start);
		parser->on_finish();
	}
	Timer::print("TOTAL   ", total);
}

void print_node_location (FILE* buffer, Ast* node) {
	if (node)
		fprintf(buffer, "@ %s:%d,%d", node->filename, node->line, node->col);
	else fprintf(buffer, "@ (null):?,?");
}

void Light_Compiler::info (Ast* node, const char* format, ...) {
	fprintf(stdout, "\n[INFO] ");

	va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
	fprintf(stdout, "\n\t");

	print_node_location(stdout, node);
	fprintf(stdout, "\n");
}

void Light_Compiler::warning (Ast* node, const char* format, ...) {
	fprintf(stdout, "\n[WARNING] ");

	va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
	fprintf(stdout, "\n\t");

	print_node_location(stdout, node);
	fprintf(stdout, "\n");
}

void Light_Compiler::error (Ast* node, const char* format, ...) {
	va_list argptr;
    va_start(argptr, format);
    v_error(node, format, argptr);
    va_end(argptr);
}

void Light_Compiler::v_error (Ast* node, const char* format, va_list argptr) {
	fprintf(stderr, "\n[ERROR] ");

    vfprintf(stderr, format, argptr);
	fprintf(stderr, "\n\t");

	print_node_location(stderr, node);
	fprintf(stderr, "\n");
	has_errors = true;
}

void Light_Compiler::error_stop (Ast* node, const char* format, ...) {
	va_list argptr;
    va_start(argptr, format);
	this->v_error(node, format, argptr);
    va_end(argptr);
	this->stop();
}

void Light_Compiler::stop () {
	fprintf(stdout, "\nStopping compilation...\n");
	exit(1);
}
