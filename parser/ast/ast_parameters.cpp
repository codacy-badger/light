#pragma once


using namespace std;

#include "ast_node.cpp"

class ASTParameters : public ASTNode {
	public:
		vector<ASTExpression*> expressions;

		void print (int tabs) {
			if (this->expressions.size() > 0) {
				auto value = this->expressions[0];
				value->print(tabs);
				for(std::vector<int>::size_type i = 1; i < this->expressions.size(); i++) {
					cout << ", ";
					this->expressions[i]->print(tabs);
				}
			}
		}
};
