#pragma once

#include "parser/pipes.hpp"
#include "parser/printer.hpp"

struct PrintPipe : Pipe {
	void onStatement(Ast_Statement* stm);
};
