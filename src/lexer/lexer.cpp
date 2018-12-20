#include "lexer/lexer.hpp"

#include "compiler.hpp"

#define LEXER_IGNORED " \n\t"

#define STRING_TOKEN(literal, length, type)										\
	if (this->buffer->is_next(literal, length))									\
	{ handle_token(type, literal); return true; }
#define CHAR_TOKEN(c, type) if (this->buffer->is_next(c))						\
	{ handle_token(type); return true; }

#define STRING_EQUAL(str1, str2) (strcmp(str1, str2) == 0)

const char* token_get_text (Token_Type type);

Lexer::Lexer (Lexer_Buffer* buffer) {
	this->buffer = buffer;
	this->parse_next();
}

bool Lexer::parse_next () {
	while (this->skip_ignored_and_comments());

	if (!this->buffer->has_next()) {
		this->next_text = NULL;
		this->next_type = TOKEN_EOF;
		return false;
	}

    STRING_TOKEN("->", 2, TOKEN_ARROW);
	STRING_TOKEN("&&", 2, TOKEN_DOUBLE_AMP);
	STRING_TOKEN("||", 2, TOKEN_DOUBLE_PIPE);
	STRING_TOKEN(">>", 2, TOKEN_RIGHT_SHIFT);
	STRING_TOKEN("<<", 2, TOKEN_LEFT_SHIFT);
	STRING_TOKEN("==", 2, TOKEN_DOUBLE_EQUAL);
	STRING_TOKEN("!=", 2, TOKEN_NOT_EQUAL);
	STRING_TOKEN(">=", 2, TOKEN_GREATER_EQUAL);
	STRING_TOKEN("<=", 2, TOKEN_LESSER_EQUAL);
	STRING_TOKEN("++", 2, TOKEN_DOUBLE_ADD);
	STRING_TOKEN("--", 2, TOKEN_DOUBLE_SUB);
	STRING_TOKEN("..", 2, TOKEN_DOUBLE_DOT);

	CHAR_TOKEN('+', TOKEN_ADD);
	CHAR_TOKEN('-', TOKEN_SUB);
	CHAR_TOKEN('*', TOKEN_MUL);
	CHAR_TOKEN('/', TOKEN_DIV);
	CHAR_TOKEN('%', TOKEN_PERCENT);
	CHAR_TOKEN('>', TOKEN_GREATER);
	CHAR_TOKEN('<', TOKEN_LESSER);
	CHAR_TOKEN('&', TOKEN_AMP);
	CHAR_TOKEN('|', TOKEN_PIPE);
	CHAR_TOKEN('^', TOKEN_CARET);
	CHAR_TOKEN('~', TOKEN_TILDE);
	CHAR_TOKEN('!', TOKEN_EXCLAMATION);
	CHAR_TOKEN('$', TOKEN_DOLLAR);
	CHAR_TOKEN('@', TOKEN_AT);
	CHAR_TOKEN('#', TOKEN_HASH);
	CHAR_TOKEN('=', TOKEN_EQUAL);
	CHAR_TOKEN(':', TOKEN_COLON);
	CHAR_TOKEN(';', TOKEN_STM_END);
    CHAR_TOKEN('.', TOKEN_DOT);
    CHAR_TOKEN(',', TOKEN_COMMA);
    CHAR_TOKEN('(', TOKEN_PAR_OPEN);
    CHAR_TOKEN(')', TOKEN_PAR_CLOSE);
    CHAR_TOKEN('{', TOKEN_BRAC_OPEN);
    CHAR_TOKEN('}', TOKEN_BRAC_CLOSE);
    CHAR_TOKEN('[', TOKEN_SQ_BRAC_OPEN);
    CHAR_TOKEN(']', TOKEN_SQ_BRAC_CLOSE);

	if (next_is_id()) {
			 if (STRING_EQUAL(next_text, "if")) 		this->next_type = TOKEN_IF;
		else if (STRING_EQUAL(next_text, "else")) 		this->next_type = TOKEN_ELSE;
		else if (STRING_EQUAL(next_text, "while")) 		this->next_type = TOKEN_WHILE;
		else if (STRING_EQUAL(next_text, "break")) 		this->next_type = TOKEN_BREAK;
		else if (STRING_EQUAL(next_text, "cast")) 		this->next_type = TOKEN_CAST;
		else if (STRING_EQUAL(next_text, "struct")) 	this->next_type = TOKEN_STRUCT;
		else if (STRING_EQUAL(next_text, "fn")) 		this->next_type = TOKEN_FUNCTION;
		else if (STRING_EQUAL(next_text, "return")) 	this->next_type = TOKEN_RETURN;
		else if (STRING_EQUAL(next_text, "import")) 	this->next_type = TOKEN_IMPORT;
		else if (STRING_EQUAL(next_text, "include")) 	this->next_type = TOKEN_INCLUDE;
		else if (STRING_EQUAL(next_text, "foreign")) 	this->next_type = TOKEN_FOREIGN;
		else if (STRING_EQUAL(next_text, "run")) 		this->next_type = TOKEN_RUN;
		else if (STRING_EQUAL(next_text, "false")) 		this->next_type = TOKEN_FALSE;
		else if (STRING_EQUAL(next_text, "true")) 		this->next_type = TOKEN_TRUE;
		else if (STRING_EQUAL(next_text, "null")) 		this->next_type = TOKEN_NULL;
		return true;
	} else if (next_is_string() || next_is_number()) return true;

	ERROR_STOP(this->buffer, "Unrecognized character: '%c'", this->buffer->peek());
	return false;
}

bool Lexer::is_next_type (Token_Type type) {
	return this->next_type == type;
}

void Lexer::skip (unsigned int count) {
	for (unsigned int i = 0; i < count; i++)
		this->parse_next();
}

const char* Lexer::text () {
	auto text = this->next_text;
	this->skip(1);
	return text;
}

bool Lexer::check_skip (Token_Type type) {
	if (this->next_type == type) {
		this->skip(1);
		return true;
	} else {
		this->report_unexpected(type);
		return false;
	}
}

bool Lexer::optional_skip (Token_Type type) {
	if (this->next_type == type) {
		this->skip(1);
		return true;
	} else return false;
}

void Lexer::report_unexpected (Token_Type expected) {
	ERROR_STOP(this->buffer, "Expected '%s', but got '%s'",
		token_get_text(expected), token_get_text(this->next_type));
}

bool Lexer::next_is_id () {
	this->next_text = this->buffer->get_next_id();
	if (this->next_text) {
		this->next_type = TOKEN_ID;
		return true;
	} else return false;
}

bool Lexer::next_is_string () {
	this->next_text = this->buffer->get_next_string();
	if (this->next_text) {
		this->next_type = TOKEN_STRING;
		return true;
	} else return false;
}

bool Lexer::next_is_number () {
	this->next_text = this->buffer->get_next_number();
	if (this->next_text) {
		this->next_type = TOKEN_NUMBER;
		return true;
	} else return false;
}

void Lexer::handle_token (Token_Type type, const char* text) {
	if (text) this->buffer->skip(strlen(text));
	else this->buffer->skip();
	this->next_text = NULL;
	this->next_type = type;
}

bool Lexer::skip_ignored_and_comments () {
	this->buffer->skip_any(LEXER_IGNORED);
    if (this->buffer->peek() == '/') {
        if (this->buffer->peek(1) == '/') {
			this->buffer->skip_until("\n");
			this->buffer->skip_any(LEXER_IGNORED);
			return true;
        } else if (this->buffer->peek(1) == '*') {
			this->buffer->skip_until("*/");
			this->buffer->skip_any(LEXER_IGNORED);
			return true;
        }
    }
	return false;
}

size_t Lexer::get_total_lines() {
	return this->buffer->location.line;
}

#define CASE_ENUM_TEXT(T, str) case T: return str;

const char* token_get_text (Token_Type type) {
	switch (type) {
		CASE_ENUM_TEXT(TOKEN_EOF, 			"<EOF>")

		CASE_ENUM_TEXT(TOKEN_ID,			"<identifier>")
		CASE_ENUM_TEXT(TOKEN_NUMBER,		"<number>")
		CASE_ENUM_TEXT(TOKEN_STRING,		"<string>")

		CASE_ENUM_TEXT(TOKEN_DOUBLE_AMP,	"&&")
		CASE_ENUM_TEXT(TOKEN_DOUBLE_PIPE,	"||")
		CASE_ENUM_TEXT(TOKEN_DOUBLE_ADD,	"++")
		CASE_ENUM_TEXT(TOKEN_DOUBLE_SUB,	"--")
		CASE_ENUM_TEXT(TOKEN_DOUBLE_EQUAL,	"==")
		CASE_ENUM_TEXT(TOKEN_NOT_EQUAL,		"!=")
		CASE_ENUM_TEXT(TOKEN_GREATER_EQUAL,	">=")
		CASE_ENUM_TEXT(TOKEN_LESSER_EQUAL,	"<=")
		CASE_ENUM_TEXT(TOKEN_RIGHT_SHIFT,	">>")
		CASE_ENUM_TEXT(TOKEN_LEFT_SHIFT,	"<<")
		CASE_ENUM_TEXT(TOKEN_ARROW,			"->")
		CASE_ENUM_TEXT(TOKEN_DOUBLE_DOT,	"..")

		CASE_ENUM_TEXT(TOKEN_IF,			"IF")
		CASE_ENUM_TEXT(TOKEN_ELSE,			"ELSE")
		CASE_ENUM_TEXT(TOKEN_WHILE,			"WHILE")
		CASE_ENUM_TEXT(TOKEN_BREAK,			"BREAK")
		CASE_ENUM_TEXT(TOKEN_CAST,			"CAST")
		CASE_ENUM_TEXT(TOKEN_STRUCT,		"STRUCT")
		CASE_ENUM_TEXT(TOKEN_FUNCTION,		"FUNCTION")
		CASE_ENUM_TEXT(TOKEN_RETURN,		"RETURN")
		CASE_ENUM_TEXT(TOKEN_IMPORT,		"IMPORT")
		CASE_ENUM_TEXT(TOKEN_INCLUDE,		"INCLUDE")
		CASE_ENUM_TEXT(TOKEN_FOREIGN,		"FOREIGN")
		CASE_ENUM_TEXT(TOKEN_RUN,			"RUN")
		CASE_ENUM_TEXT(TOKEN_FALSE,			"FALSE")
		CASE_ENUM_TEXT(TOKEN_TRUE,			"TRUE")
		CASE_ENUM_TEXT(TOKEN_NULL,			"NULL")

		CASE_ENUM_TEXT(TOKEN_EXCLAMATION,	"!")
		CASE_ENUM_TEXT(TOKEN_DOLLAR,		"$")

		CASE_ENUM_TEXT(TOKEN_AMP,			"&")
		CASE_ENUM_TEXT(TOKEN_PIPE,			"|")
		CASE_ENUM_TEXT(TOKEN_CARET,			"^")
		CASE_ENUM_TEXT(TOKEN_TILDE,			"~")
		CASE_ENUM_TEXT(TOKEN_ADD,			"+")
		CASE_ENUM_TEXT(TOKEN_SUB,			"-")
		CASE_ENUM_TEXT(TOKEN_DIV,			"/")
		CASE_ENUM_TEXT(TOKEN_MUL,			"*")
		CASE_ENUM_TEXT(TOKEN_PERCENT,		"%")
		CASE_ENUM_TEXT(TOKEN_GREATER,		">")
		CASE_ENUM_TEXT(TOKEN_LESSER,		"<")

		CASE_ENUM_TEXT(TOKEN_EQUAL,			"=")
		CASE_ENUM_TEXT(TOKEN_HASH,			"#")

		CASE_ENUM_TEXT(TOKEN_STM_END,		";")
		CASE_ENUM_TEXT(TOKEN_PAR_OPEN,		"(")
		CASE_ENUM_TEXT(TOKEN_PAR_CLOSE,		")")
		CASE_ENUM_TEXT(TOKEN_BRAC_OPEN,		"{")
		CASE_ENUM_TEXT(TOKEN_BRAC_CLOSE,	"}")
		CASE_ENUM_TEXT(TOKEN_SQ_BRAC_OPEN,	"[")
		CASE_ENUM_TEXT(TOKEN_SQ_BRAC_CLOSE,	"]")
		CASE_ENUM_TEXT(TOKEN_COLON,			":")
		CASE_ENUM_TEXT(TOKEN_COMMA,			",")
		CASE_ENUM_TEXT(TOKEN_DOT,			".")
		CASE_ENUM_TEXT(TOKEN_AT,			"@")

		default: return "--- UNDEFINED ---";
	}
}
