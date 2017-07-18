#pragma once


using namespace std;

#include "../ast_parameters.cpp"

class ASTCall : public ASTExpression {
	public:
		ASTExpression* expression = NULL;
		ASTParameters* parameters = NULL;

		void print (int tabs) {
			this->expression->print(tabs);
			cout << "(";
			this->parameters->print(tabs);
			cout << ")";
		}
};
