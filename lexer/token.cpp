#pragma once

#include <string>
#include <ostream>
#include <iostream>

#define ENUM_STR(name) case Type::name: return #name

class Token {
public:
	enum Type {
		NONE,

		EQUAL,
		ADD,
		SUB,
		DIV,
		MUL,
		LET,
		ARROW,
		TYPE,
		FUNCTION,
		STM_END,
		RETURN,
		PAR_OPEN,
		PAR_CLOSE,
		BRAC_OPEN,
		BRAC_CLOSE,
		SQ_BRAC_OPEN,
		SQ_BRAC_CLOSE,
		COLON,
		COMMA,
		DOT,
		ID,
		NUMBER,
		STRING,
	};

	static const char* typeToString (Type type) {
		switch (type) {
			ENUM_STR(NONE);

			ENUM_STR(EQUAL);
			ENUM_STR(ADD);
			ENUM_STR(SUB);
			ENUM_STR(DIV);
			ENUM_STR(MUL);
			ENUM_STR(LET);
			ENUM_STR(ARROW);
			ENUM_STR(TYPE);
			ENUM_STR(FUNCTION);
			ENUM_STR(STM_END);
			ENUM_STR(RETURN);
			ENUM_STR(PAR_OPEN);
			ENUM_STR(PAR_CLOSE);
			ENUM_STR(BRAC_OPEN);
			ENUM_STR(BRAC_CLOSE);
			ENUM_STR(SQ_BRAC_OPEN);
			ENUM_STR(SQ_BRAC_CLOSE);
			ENUM_STR(DOT);
			ENUM_STR(COMMA);
			ENUM_STR(ID);
			ENUM_STR(NUMBER);

			default: return "?";
		}
	}
};
