#pragma once

#include <iostream>

using namespace std;

class ASTId : public ASTVariable {
	public:
		string name;

		ASTId (string name = "") {
			this->name = name;
		}

		void print (int tabs) {
			cout << "[" << this->name << "]";
		}

		ASTType* getType(ParserContext* context) {
			// TODO: store variables in context to query type
			return nullptr;
		}
};
