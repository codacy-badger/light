#pragma once

#include <iostream>

using namespace std;

class ASTExpressionUnopNeg : public ASTExpressionUnop {
	public:
		void print (int tabs) {
			cout << "(-";
			this->expression->print(tabs);
			cout << ")" << endl;
		}
};
