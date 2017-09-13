#pragma once

#include <iostream>
#include <vector>

#include "ast_statement.cpp"

using namespace std;

class ASTStatements : ASTStatement {
	public:
		vector<ASTStatement*> list;

		void print (int tabs) {
			for(auto const& value: this->list) {
				value->print(tabs);
			}
		}
};
