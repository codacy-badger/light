#pragma once

#include "parser/pipes.hpp"

struct Constant_Folding : Pipe {
	void on_statement(Ast_Statement* stm);

	void fold (Ast_Statement* stm);
	void fold (Ast_Expression** stm);
	void fold (Ast_Binary** binary);
	void fold (Ast_Unary** unary);
	void fold (Ast_Type_Definition** tydef);
};
