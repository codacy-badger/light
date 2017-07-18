#pragma once


using namespace std;

#include "ast_node.cpp"
#include "ast_type.cpp"
//#include "expression/ast_expression.cpp"

class ASTDefParameter : public ASTNode {
	public:
		ASTType* type = NULL;
		string name;
		//ASTExpression* value;

		void print (int tabs) {
			cout << this->name << " as ";
			this->type->print(tabs);
			//if (this->value != NULL) this->value->print(tabs);
		}
};
