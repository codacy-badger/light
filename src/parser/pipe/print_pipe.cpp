#pragma once

#include "../pipes.cpp"
#include "parser/printer.cpp"

struct PrintPipe : Pipe {

	void onFunction (Ast_Function* fn) {
		ASTPrinter::print(fn);
		this->toNext(fn);
	}

	void onType (Ast_Type_Definition* ty) {
		ASTPrinter::print(ty);
		this->toNext(ty);
	}
};
