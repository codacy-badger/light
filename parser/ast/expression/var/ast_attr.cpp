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

		ASTType* getType(ParserContext* context) {
			// TODO: store variables in context to query type
			return nullptr;
		}
};
