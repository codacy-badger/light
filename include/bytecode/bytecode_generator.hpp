#pragma once

#include "parser/pipes.hpp"

struct Bytecode_Generator : Pipe {
	size_t global_offset;
	size_t stack_offset;

	void on_statement(Ast_Statement* stm);

    void gen (Ast_Statement* stm, vector<Instruction*>* bytecode, size_t reg);
	void gen (Ast_Block* block, vector<Instruction*>* bytecode, size_t reg);
	void gen (Ast_Return* ret, vector<Instruction*>* bytecode, size_t reg);
    void gen (Ast_Declaration* decl, vector<Instruction*>* bytecode, size_t reg);

    size_t gen (Ast_Expression* exp, vector<Instruction*>* bytecode, size_t reg);
	size_t gen (Ast_Literal* lit, vector<Instruction*>* bytecode, size_t reg);
	size_t gen (Ast_Unary* unop, vector<Instruction*>* bytecode, size_t reg);
	size_t gen (Ast_Binary* binop, vector<Instruction*>* bytecode, size_t reg);
	size_t gen (Ast_Ident* ident, vector<Instruction*>* bytecode, size_t reg);
    size_t gen (Ast_Function* fn, vector<Instruction*>* bytecode, size_t reg);
};
