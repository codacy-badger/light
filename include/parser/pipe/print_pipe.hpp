#pragma once

#include "parser/pipes.hpp"
#include "parser/printer.hpp"

struct PrintPipe : Pipe {
	void onFunction (Ast_Function* fn);
	void onType (Ast_Type_Definition* ty);
};
