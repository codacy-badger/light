#pragma once

#include <iostream>

using namespace std;

class ASTCall : public ASTExpression {
	public:
		ASTVariable* var = nullptr;
		vector<ASTExpression*> params;

		void print (int tabs) {
			cout << "(";
			if (this->var != nullptr)
				this->var->print(tabs);
			cout << " CALL ( ";
			if (params.size() > 0) {
				params[0]->print(tabs);
				for (int i = 1; i < params.size(); i++) {
					cout << ", ";
					params[i]->print(tabs);
				}
			}
			cout << " ))";
		}

		bool isConstant () {
			return false;
		}

		ASTType* getType(ParserContext* context) {
			// TODO: store variables in context to query type
			return nullptr;
		}
};
