#pragma once

#include "parser/pipes.hpp"

void Pipe::onDeclaration (Ast_Declaration* decl) {
	this->toNext(decl);
}

void Pipe::onFinish () {
	this->tryFinish();
}

void Pipe::tryFinish() {
	if (next) next->onFinish();
}

void Pipe::append (Pipe* next) {
	Pipe* last = this;
	while (last->next) last = last->next;
	last->next = next;
}
