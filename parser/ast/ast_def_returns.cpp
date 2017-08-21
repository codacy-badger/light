#pragma once


using namespace std;

#include "ast_node.cpp"
#include "ast_type.cpp"

class ASTDefReturns : public ASTNode {
	public:
		vector<ASTType*> list;

		void print (int tabs) {
			if (this->list.size() > 0) {
				auto value = this->list[0];
				value->print(tabs);
				for(std::vector<int>::size_type i = 1; i < this->list.size(); i++) {
					cout << ", ";
					this->list[i]->print(tabs);
				}
			}
		}
};
