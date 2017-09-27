#pragma once

#include "ast_expression.cpp"

using namespace std;

class ASTConst : public ASTExpression {
public:
	enum TYPE { BYTE, SHORT, INT, LONG, FLOAT, DOUBLE, STRING, COUNT };
	TYPE type = TYPE::COUNT;
	union {
		int8_t byteValue;
		int16_t shortValue;
		int32_t intValue;
		int64_t longValue;
		float floatValue;
		double doubleValue;
		char* stringValue;
	};

	ASTConst (TYPE type) {
		this->type = type;
	}

	void print (int tabs) {
		switch (type) {
			case BYTE:   cout << this->byteValue;   break;
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
