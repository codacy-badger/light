#pragma once


using namespace std;

#include "ast_node.cpp"

class ASTParameters : public ASTNode {
	public:
		vector<ASTExpression*> expressions;

		void print (int tabs) {
			for(auto const& value: this->expressions) {
				value->print(tabs);
				cout << ", ";
			}
		}
};
