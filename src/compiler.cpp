#include "compiler.hpp"

#include <stdio.h>

#include "platform.hpp"
#include "parser/pipe/symbol_resolution.hpp"
#include "parser/pipe/constant_folding.hpp"
#include "parser/pipe/type_checking.hpp"
#include "parser/pipe/foreign_function.hpp"
#include "parser/pipe/compile_constants.hpp"
#include "parser/pipe/array_attributes.hpp"

#include "bytecode/pipe/bytecode_generator.hpp"
#include "bytecode/pipe/bytecode_runner.hpp"

#define COMPILER_LOC_FORMAT "\n  LoC                        %8zd   (%5.3f ms / line)\n"
#define COMPILER_TOTAL_FORMAT "  TOTAL                      %8.6f s\n"
#define COMPILER_DONE_FORMAT "\nCompleted in %8.6f s\n"

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

void print_compiler_metrics (Parser* parser, double total_time) {
	printf("\n");
	Pipe* current_pipe = parser;
	while (current_pipe) {
		if (current_pipe->pipe_name) {
			double percent = (current_pipe->accumulated_spans * 100.0) / total_time;
			printf("  - %-25s%8.6f s (%5.2f %%)\n", current_pipe->pipe_name,
				current_pipe->accumulated_spans, percent);
		}
		current_pipe = current_pipe->next;
	}

	auto ms_per_line = (total_time * 1000) / parser->all_lines;
	printf(COMPILER_LOC_FORMAT, parser->all_lines, ms_per_line);
	printf(COMPILER_TOTAL_FORMAT, total_time);
}

void Compiler::run () {
	printf("%s v%s\n", LIGHT_NAME, LIGHT_VERSION);

	auto total = os_get_time();
	for (auto filename : this->settings->input_files) {
		printf("\n%s\n", filename);

		auto parser = new Parser();
		os_get_current_directory(parser->current_path);

		parser->append(new Foreign_Function());
		parser->append(new Compile_Constants());
		parser->append(new Symbol_Resolution());
		parser->append(new Constant_Folding());
		parser->append(new Type_Checking());
		parser->append(new Array_Attributes());

		parser->append(new Bytecode_Generator());
		parser->append(new Bytecode_Runner());

		// @Incomplete: add output pipes (DLL, EXE, etc.)

		auto start = os_get_time();
		parser->run(filename);
		parser->on_finish();

		print_compiler_metrics(parser, os_clock_stop(start));
	}

	printf(COMPILER_DONE_FORMAT, os_clock_stop(total));
}

void Compiler::stop () {
	fprintf(stdout, "\nStopping compilation...\n");
	exit(1);
}
