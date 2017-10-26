#pragma once

#include "parser/pipes.hpp"

void Pipe::append (Pipe* next) {
	Pipe* last = this;
	while (last->next) last = last->next;
	last->next = next;
}
