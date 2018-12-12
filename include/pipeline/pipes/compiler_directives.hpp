#pragma once

#include "pipeline/pipe.hpp"

#include "compiler.hpp"

struct Compiler_Directives : Pipe {
	PIPE_NAME(Compiler_Directives)

    void handle (Ast_Directive_Include** include_ptr) {
		auto include = (*include_ptr);

		Compiler::instance->modules->get_module(include->absolute_path);
		// TODO: replace directive by the code inside the module
    }

    void handle (Ast_Directive_If**) { /* TODO: implement */ }
};
