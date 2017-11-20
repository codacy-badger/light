#pragma once

#include "parser/pipes.hpp"

struct Register_Allocation : Pipe {
    int64_t current_register = 0;

    void on_statement(Ast_Statement* stm);

	void assign (Ast_Statement* stm);
	void assign (Ast_Block* block);
	void assign (Ast_Return* ret);
    void assign (Ast_Declaration* decl);

    void assign (Ast_Expression* exp);
	void assign (Ast_Literal* lit);
	void assign (Ast_Unary* unop);
	void assign (Ast_Binary* binop);
	void assign (Ast_Ident* ident);
    void assign (Ast_Function* fn);
	void assign (Ast_Function_Call* call);
    void assign (Ast_Comma_Separated_Arguments* args);
};
