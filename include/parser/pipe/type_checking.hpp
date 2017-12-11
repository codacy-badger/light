#pragma once

#include "parser/pipes.hpp"

struct Type_Checking : Pipe {
    void on_statement(Ast_Statement* stm);

    void check_type (Ast_Statement* stm);
    void check_type (Ast_Note* note);
    void check_type (Ast_Block* block);
    void check_type (Ast_If* _if);
    void check_type (Ast_While* _while);
    void check_type (Ast_Declaration* decl);
    void check_type (Ast_Return* ret);
    void check_type (Ast_Expression* exp);

	void check_type (Ast_Cast* cast);
	void check_type (Ast_Type_Definition* tydef);
	void check_type (Ast_Function_Type* ty);
	void check_type (Ast_Struct_Type* ty);
	void check_type (Ast_Pointer_Type* ty);
	void check_type (Ast_Array_Type* arr);

    void check_type (Ast_Function* func);
	void check_type (Ast_Function_Call* call);
	void check_type (Ast_Binary* binop);
	void check_type (Ast_Unary* unop);
    void check_type (Ast_Ident* ident);
    void check_type (Ast_Literal* lit);
};
