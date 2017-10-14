#pragma once

#include <iostream>

#include "parser/ast.hpp"

struct Pipe {
	Pipe* next = nullptr;

	virtual void onFunction (Ast_Function* fn);
	virtual void onType (Ast_Type_Definition* ty);
	virtual void onFinish ();

	void tryFinish();

	template <typename T>
	void toNext (T* node) {
		if (auto obj = dynamic_cast<Ast_Function*>(node)) {
			if (next) next->onFunction(obj);
		} else if (auto obj = dynamic_cast<Ast_Type_Definition*>(node)) {
			if (next) next->onType(obj);
		}
	}

	void append (Pipe* next);
};
