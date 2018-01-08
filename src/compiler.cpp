#include "compiler.hpp"

#include <stdio.h>

#include "platform.hpp"
#include "parser/pipe/symbol_resolution.hpp"
#include "parser/pipe/constant_folding.hpp"
#include "parser/pipe/unique_types.hpp"
#include "parser/pipe/type_checking.hpp"
#include "parser/pipe/foreign_function.hpp"
#include "parser/pipe/compile_constants.hpp"
#include "parser/pipe/array_attributes.hpp"
#include "parser/pipe/poly_functions.hpp"

#include "bytecode/pipe/bytecode_generator.hpp"
#include "bytecode/pipe/bytecode_runner.hpp"

Compiler::Compiler () {
	this->type_def_type = 	new Ast_Struct_Type("type");
	this->type_def_void = 	new Ast_Struct_Type("void");
	this->type_def_bool = 	new Ast_Struct_Type("bool", 1);
	this->type_def_s8 = 	new Ast_Struct_Type("s8", 1);
	this->type_def_s16 = 	new Ast_Struct_Type("s16", 2);
	this->type_def_s32 = 	new Ast_Struct_Type("s32", 4);
	this->type_def_s64 = 	new Ast_Struct_Type("s64", 8);
	this->type_def_u8 = 	new Ast_Struct_Type("u8", 1);
	this->type_def_u16 = 	new Ast_Struct_Type("u16", 2);
	this->type_def_u32 = 	new Ast_Struct_Type("u32", 4);
	this->type_def_u64 = 	new Ast_Struct_Type("u64", 8);
	this->type_def_f32 = 	new Ast_Struct_Type("f32", 4);
	this->type_def_f64 = 	new Ast_Struct_Type("f64", 8);

	// TODO: size should be platform dependendant, since it's used for indexing
	this->type_def_usize = 	this->type_def_u64;
}

void Compiler::run () {
	printf("%s v%s\n", LIGHT_NAME, LIGHT_VERSION);

	auto total = os_get_time();
	for (auto filename : this->settings->input_files) {
		printf("\n%s\n", filename);

		this->parser = new Parser(filename);
		// Mandatory
		parser->append(new Foreign_Function());
		parser->append(new Compile_Constants());
		//parser->append(new Poly_Functions());
		parser->append(new Symbol_Resolution());
		parser->append(new Constant_Folding());
		parser->append(new Unique_Types());
		parser->append(new Type_Checking());
		parser->append(new Array_Attributes());
		// Bytecode
		parser->append(new Bytecode_Generator());
		parser->append(new Bytecode_Runner());
		// Ouput
		//parser->append(new PrintPipe());

		auto start = os_get_time();
		parser->top_level_block();
		auto stop = os_clock_stop(start);
		printf("\n");
		parser->on_finish(stop);
		printf("\n  TOTAL                      %8.6fs\n", stop);
	}
	printf("\nDone                         %8.6fs\n", os_clock_stop(total));
}

void Compiler::stop () {
	fprintf(stdout, "\nStopping compilation...\n");
	exit(1);
}
