#pragma once

#include "parser/ast.hpp"

struct Pipe {
	Pipe* next = NULL;

	virtual void on_block_begin (Ast_Block* block);
	virtual void on_statement (Ast_Statement* stm);
	virtual void on_block_end (Ast_Block* block);
	virtual void on_finish ();

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

	void append (Pipe* next);
};
