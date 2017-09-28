#pragma once

#include <iostream>

using namespace std;

class ASTAttr : public ASTVariable {
	public:
		string name;
		ASTExpression* exp = nullptr;

		ASTAttr (ASTExpression* exp = nullptr) {
			this->exp = exp;
		}

		void print (int tabs) {
			cout << "[ATTR ";
			if (exp != nullptr) this->exp->print(tabs);
			cout << "." << this->name << "]";
		}

		ASTType* getType(ParserContext* context) {
			// TODO: store variables in context to query type
			return nullptr;
		}
};
