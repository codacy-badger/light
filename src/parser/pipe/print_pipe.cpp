#pragma once

#include "parser/pipe/print_pipe.hpp"

void PrintPipe::onDeclaration (Ast_Declaration* decl) {
	ASTPrinter::print(decl);
	this->toNext(decl);
}
