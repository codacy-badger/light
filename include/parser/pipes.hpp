#pragma once

#include "parser/ast.hpp"

struct Pipe {
	Pipe* next = NULL;

	virtual void on_statement (Ast_Statement* stm) {
		this->to_next(stm);
	}

	virtual void on_finish () {
		this->try_finish();
	}

	void to_next (Ast_Statement* stm) {
		if (next) next->on_statement(stm);
	}

	void try_finish() {
		if (next) next->on_finish();
	}

	void append (Pipe* next_pipe) {
		Pipe* last = this;
		while (last->next) last = last->next;
		last->next = next_pipe;
	}
};
