#pragma once

#include "parser/ast/ast_node.cpp"

using namespace std;

class ASTStatement : public ASTNode {
	public:
		void print (int tabs);
};

#include "ast_var_def.cpp"
#include "ast_statements.cpp"

void ASTStatement::print (int tabs) { /* empty */ }
