#pragma once

#include "../pipes.cpp"
#include "parser/printer.cpp"

struct PrintPipe : Pipe {

	void onFunction (ASTFunction* fn) {
		ASTPrinter::print(fn);
		this->toNext(fn);
	}

	void onType (ASTType* ty) {
		ASTPrinter::print(ty);
		this->toNext(ty);
	}
};
