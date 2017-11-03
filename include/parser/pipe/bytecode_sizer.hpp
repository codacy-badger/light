#pragma once

#include "parser/pipes.hpp"

struct Bytecode_Sizer : Pipe {
	void on_statement(Ast_Statement* stm);
};
