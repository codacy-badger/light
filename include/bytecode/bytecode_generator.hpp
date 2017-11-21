#pragma once

#include "parser/pipes.hpp"

struct Bytecode_Generator : Pipe {
	size_t global_offset;
	size_t stack_offset;

	size_t current_register = 0;

	void on_statement(Ast_Statement* stm);

    void gen (Ast_Statement* stm, vector<Instruction*>* bytecode);
	void gen (Ast_Block* block, vector<Instruction*>* bytecode);
	void gen (Ast_Return* ret, vector<Instruction*>* bytecode);
    void gen (Ast_Declaration* decl, vector<Instruction*>* bytecode);

    void gen (Ast_Expression* exp, vector<Instruction*>* bytecode, bool address = false);
	void gen (Ast_Literal* lit, vector<Instruction*>* bytecode);
	void gen (Ast_Unary* unop, vector<Instruction*>* bytecode);
	void gen (Ast_Binary* binop, vector<Instruction*>* bytecode);
	void gen (Ast_Ident* ident, vector<Instruction*>* bytecode, bool address = false);
    void gen (Ast_Function* fn, vector<Instruction*>* bytecode);
	void gen (Ast_Function_Call* call, vector<Instruction*>* bytecode);
};
