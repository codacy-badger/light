#pragma once

#include "parser/pipes.hpp"
#include "parser/printer.hpp"

struct PrintPipe : Pipe {
	void on_statement(Ast_Statement* stm);
};
