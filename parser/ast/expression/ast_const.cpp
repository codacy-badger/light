#pragma once

#include "ast_expression.cpp"

using namespace std;

class ASTConst : public ASTExpression {
public:
	enum TYPE { CHAR, SHORT, INT, LONG, FLOAT, DOUBLE, STRING, COUNT };
	TYPE type = TYPE::COUNT;
	union {
		char charValue;
		short shortValue;
		int intValue;
		long longValue;
		float floatValue;
		double doubleValue;
		char* stringValue;
	};

	ASTConst (TYPE type) {
		this->type = type;
	}

	void print (int tabs) {
		switch (type) {
			case CHAR:   cout << this->charValue;   break;
			case SHORT:  cout << this->shortValue;  break;
			case INT:    cout << this->intValue;    break;
			case LONG:   cout << this->longValue;   break;
			case FLOAT:  cout << this->floatValue;  break;
			case DOUBLE: cout << this->doubleValue; break;
			case STRING: cout << "\"" << this->stringValue << "\""; break;
			default: break;
		}
	}
};
