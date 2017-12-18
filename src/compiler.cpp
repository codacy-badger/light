#include "compiler.hpp"

#include <assert.h>
#include <stdio.h>

#include "timer.hpp"
#include "parser/pipe/symbol_resolution.hpp"
#include "parser/pipe/constant_folding.hpp"
#include "parser/pipe/unique_types.hpp"
#include "parser/pipe/type_checking.hpp"
#include "parser/pipe/foreign_function.hpp"
#include "parser/pipe/compile_constants.hpp"
#include "parser/pipe/print_pipe.hpp"

Light_Compiler* Light_Compiler::inst = NULL;

Ast_Struct_Type* create_new_primitive_type (char* name, uint16_t size = 0) {
	auto output = new Ast_Struct_Type(name, size);
	output->inferred_type = Light_Compiler::inst->type_def_type;
	output->is_primitive = true;
	return output;
}

Light_Compiler::Light_Compiler (Light_Compiler_Settings* settings) {
	this->settings = settings ? settings : new Light_Compiler_Settings();
	Light_Compiler::inst = this;

	int16_t i = 1;
	if (((int8_t *) &i)[0] == 1) {
		this->byte_order = BYTEORDER_LITTLE_ENDIAN;
	} else this->byte_order = BYTEORDER_BIG_ENDIAN;

	this->type_def_type = 	create_new_primitive_type("type");
	this->type_def_void = 	create_new_primitive_type("void");
	this->type_def_bool = 	create_new_primitive_type("bool", 1);
	this->type_def_s8 = 	create_new_primitive_type("s8", 1);
	this->type_def_s16 = 	create_new_primitive_type("s16", 2);
	this->type_def_s32 = 	create_new_primitive_type("s32", 4);
	this->type_def_s64 = 	create_new_primitive_type("s64", 8);
	this->type_def_u8 = 	create_new_primitive_type("u8", 1);
	this->type_def_u16 = 	create_new_primitive_type("u16", 2);
	this->type_def_u32 = 	create_new_primitive_type("u32", 4);
	this->type_def_u64 = 	create_new_primitive_type("u64", 8);
	this->type_def_f32 = 	create_new_primitive_type("f32", 4);
	this->type_def_f64 = 	create_new_primitive_type("f64", 8);
	// TODO: size should be platform dependendant, since it's used for indexing
	this->type_def_usize = 	create_new_primitive_type("usize", 8);

	this->types->add_cast(this->type_def_u8, this->type_def_bool, NULL, true);

	this->types->add_cast(this->type_def_u8, this->type_def_u16, NULL, true);
	this->types->add_cast(this->type_def_u8, this->type_def_u32, NULL, true);
	this->types->add_cast(this->type_def_u8, this->type_def_u64, NULL, true);
	this->types->add_cast(this->type_def_u16, this->type_def_u32, NULL, true);
	this->types->add_cast(this->type_def_u16, this->type_def_u64, NULL, true);
	this->types->add_cast(this->type_def_u32, this->type_def_u64, NULL, true);

	this->types->add_cast(this->type_def_u8, this->type_def_s16, NULL, true);
	this->types->add_cast(this->type_def_u8, this->type_def_s32, NULL, true);
	this->types->add_cast(this->type_def_u8, this->type_def_s64, NULL, true);
	this->types->add_cast(this->type_def_u16, this->type_def_s32, NULL, true);
	this->types->add_cast(this->type_def_u16, this->type_def_s64, NULL, true);
	this->types->add_cast(this->type_def_u32, this->type_def_s64, NULL, true);

	this->types->add_cast(this->type_def_s8, this->type_def_s16, NULL, true);
	this->types->add_cast(this->type_def_s8, this->type_def_s32, NULL, true);
	this->types->add_cast(this->type_def_s8, this->type_def_s64, NULL, true);
	this->types->add_cast(this->type_def_s16, this->type_def_s32, NULL, true);
	this->types->add_cast(this->type_def_s16, this->type_def_s64, NULL, true);
	this->types->add_cast(this->type_def_s32, this->type_def_s64, NULL, true);

	this->types->add_cast(this->type_def_f32, this->type_def_f64, NULL, true);
	this->types->add_cast(this->type_def_u8, this->type_def_f32, NULL, true);
	this->types->add_cast(this->type_def_u16, this->type_def_f32, NULL, true);
	this->types->add_cast(this->type_def_u32, this->type_def_f32, NULL, true);
	this->types->add_cast(this->type_def_u64, this->type_def_f32, NULL, true);
	this->types->add_cast(this->type_def_s8, this->type_def_f32, NULL, true);
	this->types->add_cast(this->type_def_s16, this->type_def_f32, NULL, true);
	this->types->add_cast(this->type_def_s32, this->type_def_f32, NULL, true);
	this->types->add_cast(this->type_def_s64, this->type_def_f32, NULL, true);
	this->types->add_cast(this->type_def_u8, this->type_def_f64, NULL, true);
	this->types->add_cast(this->type_def_u16, this->type_def_f64, NULL, true);
	this->types->add_cast(this->type_def_u32, this->type_def_f64, NULL, true);
	this->types->add_cast(this->type_def_u64, this->type_def_f64, NULL, true);
	this->types->add_cast(this->type_def_s8, this->type_def_f64, NULL, true);
	this->types->add_cast(this->type_def_s16, this->type_def_f64, NULL, true);
	this->types->add_cast(this->type_def_s32, this->type_def_f64, NULL, true);
	this->types->add_cast(this->type_def_s64, this->type_def_f64, NULL, true);

	// TODO: add array structure (length, data?)
	// TODO: improve string representation in the language (array?)
	this->type_def_string = new Ast_Pointer_Type();
	this->type_def_string->base = this->type_def_u8;
}

void Light_Compiler::run () {
	auto total = Timer::getTime();
	for (auto filename : this->settings->input_files) {
		printf("%s\n", filename);

		this->parser = new Parser(this, filename);
		// Mandatory
		parser->append(new Compile_Constants());
		parser->append(new Symbol_Resolution());
		parser->append(new Constant_Folding());
		parser->append(new Unique_Types());
		parser->append(new Type_Checking());
		parser->append(new Foreign_Function());

		// Optimizations

		// Bytecode
		parser->append(this->interp->generator);
		parser->append(this->interp->runner);

		// Bytecode Optimizations

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
	if (node && node->filename) {
		fprintf(buffer, "@ %s:%zd,%zd", node->filename, node->line, node->col);
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
