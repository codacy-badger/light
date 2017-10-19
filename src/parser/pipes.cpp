#pragma once

#include "parser/pipes.hpp"

void Pipe::onStatement(Ast_Statement* stm) {
	this->toNext(stm);
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
