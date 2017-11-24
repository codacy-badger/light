#pragma once

#include "parser/ast.hpp"

struct Pipe {
	Pipe* next = NULL;

	virtual void on_block_begin (Ast_Block* block) {
		this->try_block_begin(block);
	}

	virtual void on_statement (Ast_Statement* stm) {
		this->to_next(stm);
	}

	virtual void on_block_end (Ast_Block* block) {
		this->try_block_end(block);
	}

	virtual void on_finish () {
		this->try_finish();
	}

	void try_block_begin(Ast_Block* block) {
		if (next) next->on_block_begin(block);
	}

	void to_next (Ast_Statement* stm) {
		if (next) next->on_statement(stm);
	}

	void try_block_end(Ast_Block* block) {
		if (next) next->on_block_end(block);
	}

	void try_finish() {
		if (next) next->on_finish();
	}

	void append (Pipe* next) {
		Pipe* last = this;
		while (last->next) last = last->next;
		last->next = next;
	}
};
