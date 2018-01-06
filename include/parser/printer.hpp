#pragma once

#include "parser/ast.hpp"

struct ASTPrinter {
	static void print (Ast_Statement* stm, int tabs = 0);
	static void print (Ast_Declaration* decl, int tabs = 0);
	static void print (Ast_Block* stm, int tabs = 0);
	static void print (Ast_Note* note, int tabs = 0);
	static void print (Ast_Return* ret, int tabs = 0);

	static void print (Ast_Expression* exp, int tabs = 0, bool name_only = false);

	static void print (Ast_Binary* binop, int tabs = 0);
	static void print (Ast_Unary* unop, int tabs = 0);
	static void print (Ast_Literal* cons);

	static void print (Ast_Type_Definition* tydef, int tabs = 0, bool name_only = false);
	static void print (Ast_Struct_Type* type, int tabs = 0, bool name_only = false);
	static void print (Ast_Function_Type* type, int tabs = 0);
	static void print (Ast_Pointer_Type* type, int tabs = 0);

	static void print (Ast_Function* stm, int tabs = 0, bool name_only = false);
	static void print (Ast_Function_Call* call, int tabs = 0);
	static void print (Ast_Ident* ident);
};
