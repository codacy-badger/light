#pragma once

#include "../pipes.cpp"
#include "parser/printer.cpp"

struct PrintPipe : Pipe {

	void onFunction (ASTFunction* fn) {
		ASTPrinter::print(fn);
		this->toNext(fn);
	}

	void onType (Ast_Type_Definition* ty) {
		ASTPrinter::print(ty);
		this->toNext(ty);
	}
};
