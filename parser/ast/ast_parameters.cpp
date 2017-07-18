#pragma once


using namespace std;

#include "ast_node.cpp"

class ASTParameters : public ASTNode {
	public:
		vector<ASTExpression*> list;

		void print (int tabs) {
			for(auto const& value: this->list) {
				value.print();
				cout << endl;
			}
		}
};


