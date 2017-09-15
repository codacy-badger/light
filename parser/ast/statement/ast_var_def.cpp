#pragma once

#include <iostream>

using namespace std;

class ASTVarDef : public ASTStatement {
	public:
		ASTType* type = NULL;
		string name = "";

		ASTExpression* expression = NULL;

		void print (int tabs) {
			this->tabs(tabs);
			cout << "VARIABLE [" << this->name << "], type ";
			this->type->print(tabs);
			if (this->expression != NULL) {
				cout << ", ";
				this->expression->print(tabs);
			}
			cout << endl;
		}
};