#pragma once

#include "compiler.hpp"

#include <assert.h>
#include <stdio.h>

#include "timer.hpp"
#include "parser/pipe/symbol_resolution.hpp"
#include "parser/pipe/constant_folding.hpp"
#include "parser/pipe/unique_types.hpp"
#include "parser/pipe/type_checking.hpp"
#include "parser/pipe/foreign_function.hpp"
#include "parser/pipe/print_pipe.hpp"

#include "bytecode/pipe/bytecode_generator.hpp"
#include "bytecode/pipe/bytecode_runner.hpp"

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

void create_new_native_type (Ast_Struct_Type** target, const char* name, size_t size = 0) {
	(*target) = new Ast_Struct_Type(name, size);
	(*target)->inferred_type = Light_Compiler::inst->type_def_type;
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

	create_new_native_type(&this->type_def_type, "type");
	create_new_native_type(&this->type_def_void, "void");
	create_new_native_type(&this->type_def_s8, "s8", 1);
	create_new_native_type(&this->type_def_s16, "s16", 2);
	create_new_native_type(&this->type_def_s32, "s32", 4);
	create_new_native_type(&this->type_def_s64, "s64", 8);
	create_new_native_type(&this->type_def_u8, "u8", 1);
	create_new_native_type(&this->type_def_u16, "u16", 2);
	create_new_native_type(&this->type_def_u32, "u32", 4);
	create_new_native_type(&this->type_def_u64, "u64", 8);
	create_new_native_type(&this->type_def_f32, "f32", 4);
	create_new_native_type(&this->type_def_f64, "f64", 8);
	// TODO: add array structure (length, data?)
	// TODO: improve string representation in the language (array?)
	this->type_def_string = new Ast_Pointer_Type();
	this->type_def_string->base = this->type_def_u8;

}

void Light_Compiler::run () {
	auto total = Timer::getTime();
	for (auto filename : this->settings->input_files) {
		printf("%s\n", filename);

		auto parser = new Parser(this, filename);
		// Mandatory
		parser->append(new Symbol_Resolution());
		parser->append(new Constant_Folding());
		parser->append(new Unique_Types());
		parser->append(new Type_Checking());
		parser->append(new Foreign_Function());

		// Optimizations
		// NONE

		// Bytecode
		parser->append(new Bytecode_Generator());
		parser->append(new Bytecode_Runner());

		// Bytecode Optimizations
		// NONE

		// Ouput
		//parser->append(new PrintPipe());

		auto start = Timer::getTime();
		parser->top_level_block();
		Timer::print("\n  Parse ", start);
		parser->on_finish();
	}
	Timer::print("TOTAL   ", total);
}

void print_node_location (FILE* buffer, Ast* node) {
	if (node) {
		fprintf(buffer, "@ %s:%d,%d", node->filename, node->line, node->col);
		fprintf(stdout, "\n");
	}
}

void Light_Compiler::info (Ast* node, const char* format, ...) {
	fprintf(stdout, "\n[INFO] ");

	va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
	fprintf(stdout, "\n\t");

	print_node_location(stdout, node);
}

void Light_Compiler::warning (Ast* node, const char* format, ...) {
	fprintf(stdout, "\n[WARNING] ");

	va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
	fprintf(stdout, "\n\t");

	print_node_location(stdout, node);
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
