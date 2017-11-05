#pragma once

#include "parser/pipes.hpp"

struct Foreign_Function : Pipe {
	void on_statement(Ast_Statement* stm);
};
