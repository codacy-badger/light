#include "compiler.hpp"

#include <stdio.h>

#include "platform.hpp"
#include "parser/pipe/symbol_resolution.hpp"
#include "parser/pipe/constant_folding.hpp"
#include "parser/pipe/type_checking.hpp"
#include "parser/pipe/foreign_function.hpp"
#include "parser/pipe/import_modules.hpp"

#include "bytecode/pipe/register_allocator.hpp"
#include "bytecode/pipe/bytecode_generator.hpp"
#include "bytecode/pipe/bytecode_runner.hpp"

#define COMPILER_TOTAL_FORMAT "\n  Total Time                 %8.6fs\n"
#define COMPILER_DONE_FORMAT "\nCompleted in %8.6fs (%8.6fs)\n"

Compiler::Compiler () {
	os_get_current_directory(this->settings->initial_path);
}

void print_compiler_metrics (Parser* parser, double total_time) {
	printf("\n");
	Pipe* current_pipe = parser;
	while (current_pipe) {
		if (current_pipe->pipe_name) {
			double percent = (current_pipe->accumulated_spans * 100.0) / total_time;
			printf("  - %-25s%8.6f s (%5.2f %%)\n", current_pipe->pipe_name,
				current_pipe->accumulated_spans, percent);
			current_pipe->print_pipe_metrics();
		}
		current_pipe = current_pipe->next;
	}
	printf(COMPILER_TOTAL_FORMAT, total_time);
}

void Compiler::run () {
	printf("%s v%s\n", LIGHT_NAME, LIGHT_VERSION);

	os_get_current_directory(this->parser->current_path);

	this->parser->append(new Foreign_Function());
	this->parser->append(new Symbol_Resolution());
	this->parser->append(new Type_Checking());
	this->parser->append(new Import_Modules());
	this->parser->append(new Constant_Folding());

	this->parser->append(new Register_Allocator());
	this->parser->append(new Bytecode_Generator());
	this->parser->append(new Bytecode_Runner());

	// @Incomplete: add output pipes (DLL, EXE, etc.)
	auto totalWall = os_get_wall_time();
	auto totalUser = os_get_user_time();
	for (auto filename : this->settings->input_files) {
		printf("\n%s\n", filename);

		auto start = os_get_user_time();
		this->parser->run(filename);
		this->parser->on_finish();

		print_compiler_metrics(this->parser, os_time_user_stop(start));
	}
	printf(COMPILER_DONE_FORMAT, os_time_user_stop(totalUser), os_time_wall_stop(totalWall));
}

void Compiler::add_import (Ast_Import* import) {
	this->parser->pending_imports.push_back(import);
}

void Compiler::stop () {
	fprintf(stdout, "\nStopping compilation...\n");
	exit(1);
}
