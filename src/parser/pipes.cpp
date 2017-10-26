#pragma once

#include "parser/pipes.hpp"

void Pipe::on_block_begin () {
	this->try_block_begin();
}

void Pipe::on_statement(Ast_Statement* stm) {
	this->to_next(stm);
}

void Pipe::on_block_end (Ast_Block* block) {
	this->try_block_end(block);
}

void Pipe::on_finish () {
	this->try_finish();
}

void Pipe::append (Pipe* next) {
	Pipe* last = this;
	while (last->next) last = last->next;
	last->next = next;
}
