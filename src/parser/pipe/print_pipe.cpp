#pragma once

#include "parser/pipe/print_pipe.hpp"

void PrintPipe::on_statement(Ast_Statement* stm) {
	ASTPrinter::print(stm);
	this->to_next(stm);
}
