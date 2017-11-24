#pragma once

#include "parser/pipes.hpp"

#include "dyncall/dyncall.h"

struct Bytecode_Runner : Pipe {
    DCCallVM* vm = NULL;

    Bytecode_Runner (size_t vm_size = 64) {
        this->vm = dcNewCallVM(vm_size);
    }

    ~Bytecode_Runner () {
        dcFree(this->vm);
    }

    void on_statement(Ast_Statement* stm);

	void run (Ast_Statement* stm);
	void run (Ast_Block* block);
	void run (Ast_Return* ret);
    void run (Ast_Declaration* decl);

    void run (Ast_Expression* exp);
	void run (Ast_Literal* lit);
	void run (Ast_Unary* unop);
	void run (Ast_Binary* binop);
	void run (Ast_Ident* ident);
    void run (Ast_Function* fn);
	void run (Ast_Function_Call* call);
};
