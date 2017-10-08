#pragma once

#include <iostream>

#include "ast/ast.hpp"

struct Pipe {
	Pipe* next = nullptr;

	virtual void onFunction (ASTFunction* fn) {
		cout << "NEXT: Function -> " << fn->name << "\n";
		if (next) next->onFunction(fn);
	}

	virtual void onType (ASTType* ty) {
		if (auto prim = dynamic_cast<ASTPrimitiveType*>(ty))
			cout << "NEXT: Type -> " << prim->name << "\n";
		else if (auto stct = dynamic_cast<ASTStructType*>(ty))
			cout << "NEXT: Type -> " << stct->name << "\n";
	}

	void append (Pipe* next) {
		Pipe* last = this;
		while (last->next) last = last->next;
		last->next = next;
	}
};
