#pragma once

#include <stdint.h>

enum Token_Type : uint8_t {
	TOKEN_EOF = 0,

	TOKEN_ID,
	TOKEN_NUMBER,
	TOKEN_STRING,

	TOKEN_DOUBLE_AMP,
	TOKEN_DOUBLE_PIPE,
	TOKEN_DOUBLE_ADD,
	TOKEN_DOUBLE_SUB,
	TOKEN_DOUBLE_EQUAL,
	TOKEN_NOT_EQUAL,
	TOKEN_GREATER_EQUAL,	// 10
	TOKEN_LESSER_EQUAL,
	TOKEN_RIGHT_SHIFT,
	TOKEN_LEFT_SHIFT,
	TOKEN_DOUBLE_DOT,
	TOKEN_ARROW,

	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_WHILE,
	TOKEN_BREAK,
	TOKEN_CAST,				// 20
	TOKEN_STRUCT,
	TOKEN_FUNCTION,
	TOKEN_RETURN,
	TOKEN_IMPORT,
	TOKEN_INCLUDE,
	TOKEN_FOREIGN,
	TOKEN_RUN,
	TOKEN_FALSE,
	TOKEN_TRUE,
	TOKEN_NULL,				// 30

	TOKEN_EXCLAMATION,
	TOKEN_DOLLAR,

	TOKEN_AMP,
	TOKEN_PIPE,
	TOKEN_CARET,
	TOKEN_TILDE,
	TOKEN_ADD,
	TOKEN_SUB,
	TOKEN_DIV,
	TOKEN_MUL,				// 40
	TOKEN_PERCENT,
	TOKEN_GREATER,
	TOKEN_LESSER,

	TOKEN_EQUAL,
	TOKEN_HASH,

	TOKEN_STM_END,
	TOKEN_PAR_OPEN,
	TOKEN_PAR_CLOSE,
	TOKEN_BRAC_OPEN,
	TOKEN_BRAC_CLOSE,		// 50
	TOKEN_SQ_BRAC_OPEN,
	TOKEN_SQ_BRAC_CLOSE,
	TOKEN_COLON,
	TOKEN_COMMA,
	TOKEN_DOT,
	TOKEN_AT,
};

struct Token {
    Token_Type type;
    const char* text;
	uint16_t length;

	Location location;

	Token (Location* location, Token_Type type, const char* text = NULL, size_t text_length = 0) {
		this->type = type;
		this->text = text;

		if (text != NULL) {
			if (text_length == 0) {
				this->length = (uint16_t) strlen(text);
			} else this->length = (uint16_t) text_length;
		} else this->length = 0;

		if (location) this->location = (*location);
	}

	char* copy_text () {
		auto tmp = (char*) malloc(this->length + 1);
		memcpy(tmp, this->text, this->length);
		tmp[this->length] = '\0';
		return tmp;
	}
};
