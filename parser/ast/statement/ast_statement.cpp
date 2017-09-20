#pragma once

#include "parser/ast/ast_node.cpp"

using namespace std;

class ASTStatement : public ASTNode {
	public:
		void print (int tabs) = 0;
};

#include "ast_var_def.cpp"
#include "ast_return.cpp"
#include "ast_statements.cpp"
#include "ast_function.cpp"
