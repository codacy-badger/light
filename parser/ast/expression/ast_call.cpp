#pragma once

#include <iostream>

using namespace std;

class ASTCall : public ASTExpression {
	public:
		string name;
		vector<ASTExpression*> params;

		void print (int tabs) {
			cout << "[CALL " << this->name << "(";
			for (auto const& param: this->params) {
				param->print(tabs);
				cout << ", ";
			}
			cout << ")]";
		}
};
