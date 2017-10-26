#pragma once

#include "parser/pipes.hpp"

struct Type_Inference : Pipe {
    void on_block_begin(Ast_Block* block);
    void on_statement(Ast_Statement* stm);
    void on_block_end(Ast_Block* block);
	void on_finish ();
};
