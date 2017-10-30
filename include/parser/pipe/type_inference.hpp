#pragma once

#include "parser/pipes.hpp"

struct Type_Inference : Pipe {
    void on_statement(Ast_Statement* stm);

    void infer_type (Ast_Statement* stm);
    void infer_type (Ast_Block* block);
    void infer_type (Ast_Declaration* decl);
    void infer_type (Ast_Return* ret);
    void infer_type (Ast_Expression* exp);
    void infer_type (Ast_Literal* lit);
};
