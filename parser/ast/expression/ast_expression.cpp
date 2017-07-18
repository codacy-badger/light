#pragma once


using namespace std;

#include "../ast_node.cpp"

class ASTExpression : public ASTNode {
	public:
		void print (int tabs);
};

#include "ast_id.cpp"
#include "binop/ast_expression_binop.cpp"
#include "unop/ast_expression_unop.cpp"
#include "ast_def_function.cpp"
#include "ast_def_type.cpp"
#include "ast_subscript.cpp"
#include "ast_property.cpp"
#include "ast_call.cpp"
#include "ast_number.cpp"
#include "ast_string.cpp"

void ASTExpression::print (int tabs) { /* empty */ }
