#pragma once


using namespace std;

#include <vector>
#include "ast_statement.cpp"

class ASTStatements : ASTStatement {
	public:
		vector<ASTStatement*> list;

		void print (int tabs) {
			for(auto const& value: this->list) {
				value->print(tabs);
			}
		}
};
