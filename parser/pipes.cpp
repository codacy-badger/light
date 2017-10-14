#pragma once

#include <iostream>

#include "ast.hpp"

struct Pipe {
	Pipe* next = nullptr;

	virtual void onFunction (Ast_Function* fn) {
		this->toNext(fn);
	}

	virtual void onType (Ast_Type_Definition* ty) {
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
		if (auto obj = dynamic_cast<Ast_Function*>(node)) {
			if (next) next->onFunction(obj);
		} else if (auto obj = dynamic_cast<Ast_Type_Definition*>(node)) {
			if (next) next->onType(obj);
		}
	}

	void append (Pipe* next) {
		Pipe* last = this;
		while (last->next) last = last->next;
		last->next = next;
	}
};
