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

	static const char* to_string (Token_Type type) {
		switch (type) {
			case TOKEN_EOF:				return "<EOF>";

			case TOKEN_ID:				return "<identifier>";
			case TOKEN_NUMBER:			return "<number>";
			case TOKEN_STRING:			return "<string>";

			case TOKEN_DOUBLE_AMP:		return "&&";
			case TOKEN_DOUBLE_PIPE:		return "||";
			case TOKEN_DOUBLE_ADD:		return "++";
			case TOKEN_DOUBLE_SUB:		return "--";
			case TOKEN_DOUBLE_EQUAL:	return "==";
			case TOKEN_NOT_EQUAL:		return "!=";
			case TOKEN_GREATER_EQUAL:	return ">=";
			case TOKEN_LESSER_EQUAL:	return "<=";
			case TOKEN_RIGHT_SHIFT:		return ">>";
			case TOKEN_LEFT_SHIFT:		return "<<";
			case TOKEN_DOUBLE_DOT:		return "..";
			case TOKEN_ARROW:			return "->";

			case TOKEN_IF:				return "if";
			case TOKEN_ELSE:			return "else";
			case TOKEN_WHILE:			return "while";
			case TOKEN_BREAK:			return "break";
			case TOKEN_CAST:			return "cast";
			case TOKEN_STRUCT:			return "struct";
			case TOKEN_FUNCTION:		return "fn";
			case TOKEN_RETURN:			return "return";
			case TOKEN_IMPORT:			return "import";
			case TOKEN_INCLUDE:			return "include";
			case TOKEN_FOREIGN:			return "foreign";
			case TOKEN_RUN:				return "run";
			case TOKEN_FALSE:			return "false";
			case TOKEN_TRUE:			return "true";
			case TOKEN_NULL:			return "null";

			case TOKEN_EXCLAMATION:		return "!";
			case TOKEN_DOLLAR:			return "$";

			case TOKEN_AMP:				return "&";
			case TOKEN_PIPE:			return "|";
			case TOKEN_CARET:			return "^";
			case TOKEN_TILDE:			return "'";
			case TOKEN_ADD:				return "+";
			case TOKEN_SUB:				return "-";
			case TOKEN_DIV:				return "/";
			case TOKEN_MUL:				return "*";
			case TOKEN_PERCENT:			return "%";
			case TOKEN_GREATER:			return ">";
			case TOKEN_LESSER:			return "<";

			case TOKEN_EQUAL:			return "=";
			case TOKEN_HASH:			return "#";

			case TOKEN_STM_END:			return ";";
			case TOKEN_PAR_OPEN:		return "(";
			case TOKEN_PAR_CLOSE:		return ")";
			case TOKEN_BRAC_OPEN:		return "{";
			case TOKEN_BRAC_CLOSE:		return "}";
			case TOKEN_SQ_BRAC_OPEN:	return "[";
			case TOKEN_SQ_BRAC_CLOSE:	return "]";
			case TOKEN_COLON:			return ":";
			case TOKEN_COMMA:			return ",";
			case TOKEN_DOT:				return ".";
			case TOKEN_AT:				return "@";

			default: 					return "<?>";
		}
	}
};
