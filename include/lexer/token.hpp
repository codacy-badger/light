#pragma once

#include <stdint.h>

enum Token_Type : uint8_t {
	TOKEN_EOF 				= 0,

	TOKEN_ID,
	TOKEN_NUMBER,
	TOKEN_STRING,

	TOKEN_DOUBLE_AMP,		// &&
	TOKEN_DOUBLE_PIPE,		// ||
	TOKEN_DOUBLE_ADD,		// ++
	TOKEN_DOUBLE_SUB,		// --
	TOKEN_DOUBLE_EQUAL,		// ==
	TOKEN_NOT_EQUAL,		// !=
	TOKEN_GREATER_EQUAL,	// >=
	TOKEN_LESSER_EQUAL,		// <=
	TOKEN_RIGHT_SHIFT,		// >>
	TOKEN_LEFT_SHIFT,		// <<
	TOKEN_DOUBLE_DOT,		// ..
	TOKEN_ARROW,			// ->

	TOKEN_IF,				// IF
	TOKEN_ELSE,				// ELSE
	TOKEN_WHILE,			// WHILE
	TOKEN_BREAK,			// BREAK
	TOKEN_CAST,				// CAST
	TOKEN_STRUCT,			// STRUCT
	TOKEN_FUNCTION,			// FUNCTION
	TOKEN_RETURN,			// RETURN
	TOKEN_IMPORT,			// IMPORT
	TOKEN_INCLUDE,			// INCLUDE
	TOKEN_FOREIGN,			// FOREIGN
	TOKEN_RUN,				// RUN
	TOKEN_FALSE,			// FALSE
	TOKEN_TRUE,				// TRUE
	TOKEN_NULL,				// NULL

	TOKEN_EXCLAMATION		= '!',
	TOKEN_DOLLAR			= '$',

	TOKEN_AMP				= '&',
	TOKEN_PIPE				= '|',
	TOKEN_CARET				= '^',
	TOKEN_TILDE				= '~',
	TOKEN_ADD				= '+',
	TOKEN_SUB				= '-',
	TOKEN_DIV				= '/',
	TOKEN_MUL				= '*',
	TOKEN_PERCENT			= '%',
	TOKEN_GREATER			= '>',
	TOKEN_LESSER			= '<',

	TOKEN_EQUAL				= '=',
	TOKEN_HASH				= '#',

	TOKEN_STM_END			= ';',
	TOKEN_PAR_OPEN			= '(',
	TOKEN_PAR_CLOSE			= ')',
	TOKEN_BRAC_OPEN			= '{',
	TOKEN_BRAC_CLOSE		= '}',
	TOKEN_SQ_BRAC_OPEN		= '[',
	TOKEN_SQ_BRAC_CLOSE		= ']',
	TOKEN_COLON				= ':',
	TOKEN_COMMA				= ',',
	TOKEN_DOT				= '.',
	TOKEN_AT				= '@',
};

struct Token {
    Token_Type type;
    const char* text;
	uint16_t length;

	Location location;

	Token (Location* location, Token_Type type, const char* text = NULL, uint16_t text_length = 0) {
		this->type = type;
		this->text = text;

		if (text != NULL) {
			if (text_length == 0) {
				this->length = (uint16_t) strlen(text);
			} else this->length = text_length;
		} else this->length = 0;

		if (location) this->location = (*location);
	}
};
