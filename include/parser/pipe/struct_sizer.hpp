#pragma once

#include "parser/pipes.hpp"

struct Struct_Sizer : Pipe {
	void on_statement(Ast_Statement* stm);
};
