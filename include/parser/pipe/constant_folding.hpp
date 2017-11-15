#pragma once

#include "parser/pipes.hpp"

struct Constant_Folding : Pipe {
	void on_statement(Ast_Statement* stm);

	void fold (Ast_Statement* stm);
	void fold (Ast_Declaration* stm);
	void fold (Ast_Block* stm);
	void fold (Ast_Return* ret);

	void fold (Ast_Expression** stm);
	void fold (Ast_Binary** binary);
	void fold (Ast_Unary** unary);
	void fold (Ast_Function** stm);
	void fold (Ast_Function_Call** call);

	void fold (Ast_Type_Definition** tydef);
	void fold (Ast_Struct_Type** _struct);
	void fold (Ast_Pointer_Type** ptr_type);
	void fold (Ast_Function_Type** fn_type);
};
