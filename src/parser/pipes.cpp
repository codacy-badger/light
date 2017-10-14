#pragma once

#include "parser/pipes.hpp"

void Pipe::onFunction (Ast_Function* fn) {
	this->toNext(fn);
}

void Pipe::onType (Ast_Type_Definition* ty) {
	this->toNext(ty);
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
