#pragma once


using namespace std;

class ASTProperty : public ASTExpression {
	public:
		ASTExpression* expression = NULL;
		ASTId* property = NULL;

		void print (int tabs) {
			cout << "(";
			this->expression->print(tabs);
			cout << ".";
			this->property->print(tabs);
			cout << ")";
		}
};
