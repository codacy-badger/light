#pragma once

#include <iostream>

using namespace std;

class ASTExpressionBinopSub : public ASTExpressionBinop {
	public:
		void print (int tabs) {
			cout << "(";
			this->lhs->print(tabs);
			cout << " - ";
			this->rhs->print(tabs);
			cout << ")";
		}
};
