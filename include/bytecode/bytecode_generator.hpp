#pragma once

#include "parser/pipes.hpp"

struct Bytecode_Generator : Pipe {
	void on_statement(Ast_Statement* stm);

    void gen (Ast_Statement* stm, vector<Instruction*>* bytecode = NULL, size_t reg = 0);
	void gen (Ast_Block* block, vector<Instruction*>* bytecode = NULL, size_t reg = 0);
    void gen (Ast_Declaration* decl, vector<Instruction*>* bytecode = NULL, size_t reg = 0);

    size_t gen (Ast_Expression* exp, vector<Instruction*>* bytecode = NULL, size_t reg = 0);
	size_t gen (Ast_Literal* lit, vector<Instruction*>* bytecode = NULL, size_t reg = 0);
	size_t gen (Ast_Unary* unop, vector<Instruction*>* bytecode, size_t reg);
	size_t gen (Ast_Binary* binop, vector<Instruction*>* bytecode, size_t reg);
    size_t gen (Ast_Function* fn, vector<Instruction*>* bytecode = NULL, size_t reg = 0);
};
