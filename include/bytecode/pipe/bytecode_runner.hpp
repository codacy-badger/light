#pragma once

#include "parser/pipes.hpp"

struct Bytecode_Runner : Pipe {
    void on_statement(Ast_Statement* stm);

	void run (Ast_Statement* stm);
    void run (Ast_Declaration* decl);
};
