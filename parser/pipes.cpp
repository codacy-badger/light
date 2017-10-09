#pragma once

#include <iostream>

#include "ast/ast.hpp"

struct Pipe {
	Pipe* next = nullptr;

	virtual void onFunction (ASTFunction* fn) {
		this->toNext(fn);
	}

	virtual void onType (ASTType* ty) {
		this->toNext(ty);
	}

	virtual void onFinish () {
		if (next) next->onFinish();
	}

	void toNext (ASTFunction* fn) {
		if (next) next->onFunction(fn);
	}

	void toNext (ASTType* ty) {
		if (next) next->onType(ty);
	}

	void append (Pipe* next) {
		Pipe* last = this;
		while (last->next) last = last->next;
		last->next = next;
	}
};
