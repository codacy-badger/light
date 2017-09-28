#pragma once

#include <iostream>

using namespace std;

class ASTCall : public ASTExpression {
	public:
		ASTVariable* var = nullptr;
		vector<ASTExpression*> params;

		bool isConstant () {
			return false;
		}

		ASTType* getType(ParserContext* context) {
			// TODO: store variables in context to query type
			return nullptr;
		}
};
