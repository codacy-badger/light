#pragma once

#include "parser/pipe/print_pipe.hpp"

void PrintPipe::onFunction (Ast_Function* fn) {
	ASTPrinter::print(fn);
	this->toNext(fn);
}

void PrintPipe::onType (Ast_Type_Definition* ty) {
	ASTPrinter::print(ty);
	this->toNext(ty);
}
