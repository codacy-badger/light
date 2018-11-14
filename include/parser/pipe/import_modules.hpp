#pragma once

#include "pipeline/pipe.hpp"

struct Import_Modules : Pipe {
	PIPE_NAME(Import_Modules)

	void handle (Ast_Import** import_ptr) {
		auto import = (*import_ptr);

		this->stop_processing = true;

		assert(Compiler::instance->pipeline);
		auto pipeline = Compiler::instance->pipeline;

		auto literal = static_cast<Ast_Literal*>(import->target);

		import->absolute_path = (char*) malloc(MAX_PATH_LENGTH);
		os_get_absolute_path(literal->string_value, import->absolute_path);

		for (auto imported_file : pipeline->imported_files) {
			if (strcmp(imported_file, import->absolute_path) == 0) {
				free(import->absolute_path);
				import->absolute_path = NULL;
				return;
			}
		}

		Compiler::instance->add_import(import);
	}
};
