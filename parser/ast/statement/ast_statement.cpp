#pragma once


using namespace std;

#include "../ast_node.cpp"

class ASTStatement : public ASTNode {
	public:
		void print (int tabs);
};

#include "ast_statement_def_variable.cpp"
#include "ast_statement_return.cpp"
#include "ast_statements.cpp"

void ASTStatement::print (int tabs) {
	if(ASTStatementDefVariable* v = dynamic_cast<ASTStatementDefVariable*>(this)) {
		v->print(tabs);
	} else if(ASTStatements* v = dynamic_cast<ASTStatements*>(this)) {
		v->print(tabs);
	}
}
