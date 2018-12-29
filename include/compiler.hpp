#pragma once

#include "compiler_settings.hpp"

#include "modules.hpp"
#include "code_input.hpp"
#include "pipeline/pipeline.hpp"

#include "bytecode/interpreter.hpp"
#include "ast/parser.hpp"
#include "ast/types.hpp"

#include <queue>

using namespace std;

struct Compiler {
	Compiler_Settings* settings = new Compiler_Settings();

	queue<Code_Input*> code_sources;

	Interpreter* interp = new Interpreter();
	Types* types = new Types();
	Modules* modules = new Modules(this);

	Compiler (int argc = 0, char** argv = NULL);

	void run ();
	void quit ();

	static Compiler* inst;
};
