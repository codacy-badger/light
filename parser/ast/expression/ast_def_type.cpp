#pragma once

using namespace std;

#include "../ast_def_parameters.cpp"
#include "../statement/ast_statement.cpp"

class ASTDefType : public ASTExpression {
	public:
		ASTDefParameters* parameters = NULL;
		ASTStatement* body = NULL;

		void print (int tabs) {
			this->tabs(tabs);
			cout << "TYPE (";
			this->parameters->print(tabs);
			cout << ")\n";
			body->print(tabs + 1);
		}
};
