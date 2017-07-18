#pragma once


using namespace std;

#include "ast_node.cpp"
#include "ast_type.cpp"

class ASTDefReturns : public ASTNode {
	public:
		vector<ASTType*> list;

		void print (int tabs) {
			for(auto const& value: this->list) {
				value->print(tabs);
				cout << ", ";
			}
		}
};
