#include "lexer/lexer.hpp"

#include "compiler.hpp"

#define ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
#define DIGIT(c) (c >= '0' && c <= '9')
#define ALPHANUM(c) (ALPHA(c) || DIGIT(c))
#define LEXER_IGNORED " \n\t"

#define STRING_TOKEN(literal, type) if (this->buffer->is_next(literal)) { handle_token(type, literal); return true; }
#define CHAR_TOKEN(c, type) if (this->buffer->is_next(c)) { handle_token(type); return true; }

#define STRING_EQUAL(str1, str2) (strcmp(str1, str2) == 0)

const char* token_get_text (Token_Type type);

Lexer::Lexer (FILE* file, const char* filename) {
	this->buffer = new Buffer(file, filename);
	this->parse_next();
}

Lexer::Lexer (Buffer* buffer) {
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

    STRING_TOKEN("->", TOKEN_ARROW);
	STRING_TOKEN("&&", TOKEN_DOUBLE_AMP);
	STRING_TOKEN("||", TOKEN_DOUBLE_PIPE);
	STRING_TOKEN(">>", TOKEN_RIGHT_SHIFT);
	STRING_TOKEN("<<", TOKEN_LEFT_SHIFT);
	STRING_TOKEN("==", TOKEN_DOUBLE_EQUAL);
	STRING_TOKEN("!=", TOKEN_NOT_EQUAL);
	STRING_TOKEN(">=", TOKEN_GREATER_EQUAL);
	STRING_TOKEN("<=", TOKEN_LESSER_EQUAL);
	STRING_TOKEN("++", TOKEN_DOUBLE_ADD);
	STRING_TOKEN("--", TOKEN_DOUBLE_SUB);
	STRING_TOKEN("..", TOKEN_DOUBLE_DOT);

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
			 if (STRING_EQUAL(next_text, "if")) 	this->next_type = TOKEN_IF;
		else if (STRING_EQUAL(next_text, "else")) 	this->next_type = TOKEN_ELSE;
		else if (STRING_EQUAL(next_text, "while")) 	this->next_type = TOKEN_WHILE;
		else if (STRING_EQUAL(next_text, "break")) 	this->next_type = TOKEN_BREAK;
		else if (STRING_EQUAL(next_text, "cast")) 	this->next_type = TOKEN_CAST;
		else if (STRING_EQUAL(next_text, "struct")) this->next_type = TOKEN_STRUCT;
		else if (STRING_EQUAL(next_text, "fn")) 	this->next_type = TOKEN_FUNCTION;
		else if (STRING_EQUAL(next_text, "return")) this->next_type = TOKEN_RETURN;
		else if (STRING_EQUAL(next_text, "import")) this->next_type = TOKEN_IMPORT;
		return true;
	} else if (next_is_string() || next_is_number()) return true;

	report_error_stop(&this->buffer->location, "Unrecognized token: '%d'\n",
		this->buffer->peek());
	return false;
}

bool Lexer::is_next_type (Token_Type type) {
	return this->next_type == type;
}

void Lexer::skip (unsigned int count) {
	for (unsigned int i = 0; i < count; i++)
		this->parse_next();
}

char* Lexer::text () {
	char* text = this->next_text;
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
	if (expected != TOKEN_EOF) {
		report_error_stop(&this->buffer->location, "Parser: Expected '%s', but got '%s'",
			token_get_text(expected), token_get_text(this->next_type));
	} else {
		report_error_stop(&this->buffer->location, "Parser: Unexpected token '%s'",
			token_get_text(this->next_type));
	}
}

#define LEXER_BUFFER_SIZE 128
char _buffer[LEXER_BUFFER_SIZE];

bool Lexer::next_is_id () {
	char c = this->buffer->peek();
    if (ALPHA(c)) {
		this->next_type = TOKEN_ID;
		size_t count = 0;
        while (ALPHANUM(c)) {
			_buffer[count++] = c;
			assert(count < LEXER_BUFFER_SIZE);
			this->buffer->skip();
	        c = this->buffer->peek();
        }
		_buffer[count] = 0;
		this->next_text = _strdup(_buffer);
        return true;
    }
    return false;
}

bool Lexer::next_is_string () {
	char c = this->buffer->peek();
    if (c == '"') {
		this->buffer->skip();

		size_t count = 0;
		c = this->buffer->next();
		while (c != '"') {
			if (c == '\\') {
				c = this->buffer->next();
				switch (c) {
					case 'n': _buffer[count++] = '\n'; break;
					case 't': _buffer[count++] = '\t'; break;
					default:  _buffer[count++] = c;    break;
				}
			} else _buffer[count++] = c;
			c = this->buffer->next();
		}
		_buffer[count] = 0;

        this->next_type = TOKEN_STRING;
		this->next_text = _strdup(_buffer);
        return true;
    } else return false;
}

bool Lexer::next_is_number (size_t count) {
	char c = this->buffer->peek();
	char c2 = this->buffer->peek(1);
	if (c == '0' && (c2 == 'x' || c2 == 'b')) {
		_buffer[count++] = '0';
		_buffer[count++] = c2;
		this->buffer->skip(2);
		return this->next_is_number(count);
	} else {
	    if (c == '+' || c == '-') {
	        _buffer[count++] = c;
			this->buffer->skip();
	        c = this->buffer->peek();
	    }
	    if (DIGIT(c) || c == '.') {
	        while (DIGIT(c)) {
				_buffer[count++] = c;
				this->buffer->skip();
		        c = this->buffer->peek();
	        }
	        if (c == '.') {
	            _buffer[count++] = c;
	            this->buffer->skip();
	            c = this->buffer->peek();
	            while (DIGIT(c)) {
		            _buffer[count++] = c;
		            this->buffer->skip();
		            c = this->buffer->peek();
	            }
	        }
			_buffer[count] = 0;

	        this->next_type = TOKEN_NUMBER;
			this->next_text = _strdup(_buffer);
	        return true;
	    }
	}
    return false;
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

		CASE_ENUM_TEXT(TOKEN_EXCLAMATION,	"!")

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
