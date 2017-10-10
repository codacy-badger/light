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
		this->tryFinish();
	}

	void tryFinish() {
		if (next) next->onFinish();
	}

	template <typename T>
	void toNext (T* node) {
		if (auto obj = dynamic_cast<ASTFunction*>(node)) {
			if (next) next->onFunction(obj);
		} else if (auto obj = dynamic_cast<ASTType*>(node)) {
			if (next) next->onType(obj);
		}
	}

	void append (Pipe* next) {
		Pipe* last = this;
		while (last->next) last = last->next;
		last->next = next;
	}
};
