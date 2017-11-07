#pragma once

#include "parser/pipes.hpp"

struct Bytecode_Generator : Pipe {
	void on_statement(Ast_Statement* stm);

    void gen (Ast_Statement* stm);
    void gen (Ast_Declaration* decl);

    void gen (Ast_Expression* exp);
    void gen (Ast_Function* fn);
};
