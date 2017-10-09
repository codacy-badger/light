#pragma once

#include <iostream>

#include "ast/ast.hpp"

struct Pipe {
	Pipe* next = nullptr;

	virtual void onFunction (ASTFunction* fn) {
		if (next) next->onFunction(fn);
	}

	virtual void onType (ASTType* ty) {
		if (next) next->onType(ty);
	}

	virtual void onFinish () {
		if (next) next->onFinish();
	}

	void append (Pipe* next) {
		Pipe* last = this;
		while (last->next) last = last->next;
		last->next = next;
	}
};
