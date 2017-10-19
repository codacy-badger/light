#pragma once

#include "parser/pipe/print_pipe.hpp"

void PrintPipe::onStatement(Ast_Statement* stm) {
	ASTPrinter::print(stm);
	this->toNext(stm);
}
