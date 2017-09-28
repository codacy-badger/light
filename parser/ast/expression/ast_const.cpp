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

	bool isConstant () {
		return true;
	}

	ASTType* getType(ParserContext* context) {
		switch (type) {
			case BYTE:   	return nullptr;
			case SHORT:  	return nullptr;
			case INT:    	return context->getType("i32");
			case LONG:   	return nullptr;
			case FLOAT:  	return nullptr;
			case DOUBLE: 	return nullptr;
			case STRING: 	return nullptr;
			default:		return nullptr;
		}
	}
};
