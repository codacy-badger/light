#pragma once

#include <iostream>

#include "parser/ast.hpp"

struct Pipe {
	Pipe* next = NULL;

	virtual void onDeclaration (Ast_Declaration* decl);
	virtual void onFinish ();

	void tryFinish();

	void toNext (Ast_Declaration* decl) {
		if (next) next->onDeclaration(decl);
	}

	void append (Pipe* next);
};
