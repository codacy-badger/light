#pragma once

#include <iostream>

#include "parser/ast.hpp"

struct Pipe {
	Pipe* next = NULL;

	virtual void onStatement (Ast_Statement* stm);
	virtual void onFinish ();

	void tryFinish();

	void toNext (Ast_Statement* stm) {
		if (next) next->onStatement(stm);
	}

	void append (Pipe* next);
};
