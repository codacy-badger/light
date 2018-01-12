#pragma once

#include "parser/pipes.hpp"

#include "compiler.hpp"

struct Import_Modules : Pipe {
	PIPE_NAME(Import_Modules)

	void on_statement(Ast_Statement** stm) {
	    if ((*stm)->stm_type == AST_STATEMENT_IMPORT) {
			auto start = os_get_time();
			this->handle(reinterpret_cast<Ast_Import**>(stm));
			this->accumulated_spans += os_clock_stop(start);
		} else this->to_next(stm);
	}

	void handle (Ast_Import** import_ptr) {
		auto import = (*import_ptr);

		this->remove_stm_from_block = true;

		assert(g_compiler->parser);
		auto parser = g_compiler->parser;

		auto literal = static_cast<Ast_Literal*>(import->target);

		auto abs_path = (char*) malloc(MAX_PATH_LENGTH);
		os_get_absolute_path(literal->string_value, abs_path, NULL);

		for (auto imported_file : parser->imported_files) {
			if (strcmp(imported_file, abs_path) == 0)
				return;
		}

		g_compiler->add_import(import);
	}
};
