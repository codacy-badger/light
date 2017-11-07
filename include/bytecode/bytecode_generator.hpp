#pragma once

#include "parser/pipes.hpp"

struct Bytecode_Generator : Pipe {
	void on_statement(Ast_Statement* stm);

    void gen (Ast_Statement* stm, vector<Instruction*>* bytecode = NULL);
	void gen (Ast_Block* block, vector<Instruction*>* bytecode = NULL);
    void gen (Ast_Declaration* decl, vector<Instruction*>* bytecode = NULL);

    void gen (Ast_Expression* exp, vector<Instruction*>* bytecode = NULL);
    void gen (Ast_Function* fn, vector<Instruction*>* bytecode = NULL);
};
