#pragma once

using namespace std;

#include "../ast_def_returns.cpp"
#include "../ast_def_parameters.cpp"
#include "../statement/ast_statement.cpp"

class ASTDefFunction : public ASTExpression {
	public:
		ASTDefParameters* parameters = NULL;
		ASTDefReturns* returns = NULL;
		ASTStatement* body = NULL;

		void print (int tabs) {
			this->tabs(tabs);
			cout << "FUNCTION (";
			this->parameters->print(tabs);
			cout << ") -> (";
			this->returns->print(tabs);
			cout << ")\n";
			body->print(tabs + 1);
		}
};
