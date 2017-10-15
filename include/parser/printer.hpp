#pragma once

#include "parser/ast.hpp"

struct ASTPrinter {
	static void print (Ast_Statement* stm, int tabs = 0);
	static void print (Ast_Variable* stm, int tabs = 0, bool nameOnly = false);
	static void print (Ast_Block* stm, int tabs = 0);
	static void print (Ast_Function* stm, int tabs = 0, bool nameOnly = false);
	static void print (Ast_Return* ret, int tabs = 0);
	static void print (Ast_Type_Definition* type, int tabs = 0, bool nameOnly = false);
	static void print (Ast_Primitive_Type* type, int tabs = 0, bool nameOnly = false);
	static void print (Ast_Struct_Type* type, int tabs = 0, bool nameOnly = false);
	static void print (Ast_Pointer_Type* type, int tabs = 0, bool nameOnly = false);
	static void print (Ast_Function_Type* type, int tabs = 0, bool nameOnly = false);
	static void print (Ast_Expression* exp, int tabs = 0);
	static void print (AST_Binary* binop, int tabs = 0);
	static void print (AST_Unary* unop, int tabs = 0);
	static void print (Ast_Literal* cons, int tabs = 0);
	static void print (AST_Ref* ref, int tabs = 0);
	static void print (Ast_Deref* deref, int tabs = 0);
	static void print (Ast_Function_Call* call, int tabs = 0);
	static void print (Ast_Attribute* attr, int tabs = 0);
	static void print (Ast_Unresolved* unres, int tabs = 0);

	static void _tabs (int count);
	static void _panic (const char* message);
};
