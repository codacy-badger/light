#pragma once

struct ASTValue : ASTExpression {
};

struct ASTCall : ASTValue {
	ASTExpression* fn;
	std::vector<ASTExpression*> params;

	bool isConstant () {
		return false;
	}

	ASTType* getType() {
		if (auto _fn = dynamic_cast<ASTFunction*>(fn)) {
			return _fn->type->retType;
		} else return nullptr;
	}
};

struct ASTLiteral : ASTValue {
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

	ASTLiteral (TYPE type) {
		this->type = type;
	}

	bool isConstant () {
		return true;
	}

	ASTType* getType() {
		switch (type) {
			case BYTE:   	return nullptr;
			case SHORT:  	return nullptr;
			case INT:    	return ASTPrimitiveType::_i32;
			case LONG:   	return nullptr;
			case FLOAT:  	return nullptr;
			case DOUBLE: 	return nullptr;
			case STRING: 	return nullptr;
			default:		return nullptr;
		}
	}
};

#include "ast_memory.cpp"
