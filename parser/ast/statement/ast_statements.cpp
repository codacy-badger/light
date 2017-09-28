#pragma once

#include <iostream>
#include <vector>

using namespace std;

class ASTStatements : public ASTStatement {
	public:
		vector<ASTStatement*> list;

		void print (int tabs) {
			for(auto const& value: this->list) {
				value->print(tabs);
			}
		}
};
