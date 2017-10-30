#pragma once

#include "parser/pipes.hpp"

struct Type_Checking : Pipe {
    void on_statement(Ast_Statement* stm);

    void check_type (Ast_Statement* stm);
    void check_type (Ast_Block* block);
    void check_type (Ast_Declaration* decl);
    void check_type (Ast_Return* ret);
    void check_type (Ast_Expression* exp);
    void check_type (Ast_Function* func);
    void check_type (Ast_Literal* lit);
};
