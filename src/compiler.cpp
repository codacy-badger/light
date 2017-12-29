#include "compiler.hpp"

#include <stdio.h>

#include "timer.hpp"
#include "parser/pipe/symbol_resolution.hpp"
#include "parser/pipe/constant_folding.hpp"
#include "parser/pipe/unique_types.hpp"
#include "parser/pipe/type_checking.hpp"
#include "parser/pipe/foreign_function.hpp"
#include "parser/pipe/compile_constants.hpp"
#include "parser/pipe/array_attributes.hpp"
#include "parser/pipe/print_pipe.hpp"

Ast_Struct_Type* create_new_primitive_type (Compiler* compiler, char* name, uint16_t size = 0) {
	auto output = new Ast_Struct_Type(name, size);
	output->inferred_type = compiler->type_def_type;
	output->is_primitive = true;
	return output;
}

Compiler::Compiler () {
	this->type_def_type = 	create_new_primitive_type(this, "type");
	this->type_def_void = 	create_new_primitive_type(this, "void");
	this->type_def_bool = 	create_new_primitive_type(this, "bool", 1);
	this->type_def_s8 = 	create_new_primitive_type(this, "s8", 1);
	this->type_def_s16 = 	create_new_primitive_type(this, "s16", 2);
	this->type_def_s32 = 	create_new_primitive_type(this, "s32", 4);
	this->type_def_s64 = 	create_new_primitive_type(this, "s64", 8);
	this->type_def_u8 = 	create_new_primitive_type(this, "u8", 1);
	this->type_def_u16 = 	create_new_primitive_type(this, "u16", 2);
	this->type_def_u32 = 	create_new_primitive_type(this, "u32", 4);
	this->type_def_u64 = 	create_new_primitive_type(this, "u64", 8);
	this->type_def_f32 = 	create_new_primitive_type(this, "f32", 4);
	this->type_def_f64 = 	create_new_primitive_type(this, "f64", 8);

	// TODO: size should be platform dependendant, since it's used for indexing
	this->type_def_usize = 	this->type_def_u64;
}

void Compiler::run () {
	auto total = Timer::getTime();
	for (auto filename : this->settings->input_files) {
		this->parser = new Parser(filename);
		// Mandatory
		parser->append(new Compile_Constants());
		parser->append(new Symbol_Resolution());
		parser->append(new Constant_Folding());
		parser->append(new Unique_Types());
		parser->append(new Type_Checking());
		parser->append(new Array_Attributes());
		parser->append(new Foreign_Function());
		// Bytecode
		parser->append(this->interp->generator);
		parser->append(this->interp->runner);
		// Ouput
		//parser->append(new PrintPipe());

		auto start = Timer::getTime();
		parser->top_level_block();
		Timer::print("\n  Parse %8.6fs\n", start);
		parser->on_finish();
	}
	Timer::print("TOTAL   %8.6fs\n", total);
}

void Compiler::stop () {
	fprintf(stdout, "\nStopping compilation...\n");
	exit(1);
}
