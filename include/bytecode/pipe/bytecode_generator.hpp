#pragma once

#include "parser/pipes.hpp"

struct Inst_Jump;

struct Bytecode_Generator : Pipe {
	size_t global_offset;
	size_t stack_offset;

	vector<Instruction*>* bytecode = NULL;
	uint8_t current_register = 0;

	vector<Inst_Jump*> pending_breaks;

	void on_statement(Ast_Statement* stm);

	void add_instruction (Ast* node, Instruction* intruction);

    void gen (Ast_Statement* stm);
	void gen (Ast_Return* ret);
	void gen (Ast_If* _if);
	void gen (Ast_While* _while);
	void gen (Ast_Break* _break);
    void fill (Ast_Function* fn);
    void gen (Ast_Declaration* decl);

    void gen (Ast_Expression* exp, bool left_value = false);
	void gen (Ast_Cast* cast);
	void gen (Ast_Literal* lit);
	void gen (Ast_Unary* unop, bool left_value = false);
	void gen (Ast_Binary* binop, bool left_value = false);
	void gen (Ast_Ident* ident, bool left_value = false);
    void gen (Ast_Function* fn);
	void gen (Ast_Function_Call* call);

	void update_pending_breaks ();
};
