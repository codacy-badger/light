#pragma once


using namespace std;

#include "ast_node.cpp"
#include "ast_def_parameter.cpp"

class ASTDefParameters : public ASTNode {
	public:
		vector<ASTDefParameter*> list;

		void print (int tabs) {
			for(auto const& value: this->list) {
				value->print(tabs);
				cout << ", ";
			}
		}
};
