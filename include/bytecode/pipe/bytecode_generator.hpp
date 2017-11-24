#pragma once

#include "parser/pipes.hpp"

struct Bytecode_Generator : Pipe {
	size_t global_offset;
	size_t stack_offset;

	vector<Instruction*>* bytecode = NULL;
	size_t current_register = 0;

	void on_statement(Ast_Statement* stm);

    void gen (Ast_Statement* stm);
	void gen (Ast_Block* block);
	void gen (Ast_Return* ret);
    void gen (Ast_Declaration* decl);

    void gen (Ast_Expression* exp, bool address = false);
	void gen (Ast_Literal* lit);
	void gen (Ast_Unary* unop);
	void gen (Ast_Binary* binop);
	void gen (Ast_Ident* ident, bool address = false);
    void gen (Ast_Function* fn);
	void gen (Ast_Function_Call* call);
};