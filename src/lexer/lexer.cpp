#include "lexer/lexer.hpp"

#include "compiler.hpp"

#define ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
#define DIGIT(c) (c >= '0' && c <= '9')
#define ALPHANUM(c) (ALPHA(c) || DIGIT(c))
#define LEXER_IGNORED " \n\t"

#define LITERAL_TOKEN(literal, type) if (this->buffer->is_next(literal))	{	\
	handleToken(type, literal); return true; }
#define FUNCTION_TOKEN(func) if (func()) return true;

Lexer::Lexer (const char* filename, Lexer* parent) {
	this->parent = parent;
	this->buffer = new Buffer(filename);
	if (this->buffer->is_valid()) this->parse_next();
}

Lexer::Lexer (Buffer* buffer, Lexer* parent) {
	this->parent = parent;
	this->buffer = buffer;
	this->parse_next();
}

bool Lexer::parse_next () {
	while (this->skip_ignored_and_comments());

	if (!this->buffer->hasNext()) {
		this->nextText = NULL;
		this->nextType = TOKEN_EOF;
		return false;
	}

    LITERAL_TOKEN("->", TOKEN_ARROW);

	LITERAL_TOKEN("&&", TOKEN_DOUBLE_AMP);
	LITERAL_TOKEN("||", TOKEN_DOUBLE_PIPE);

	LITERAL_TOKEN("&", TOKEN_AMP);
	LITERAL_TOKEN("|", TOKEN_PIPE);
	LITERAL_TOKEN("^", TOKEN_CARET);
	LITERAL_TOKEN("~", TOKEN_TILDE);
	LITERAL_TOKEN(">>", TOKEN_RIGHT_SHIFT);
	LITERAL_TOKEN("<<", TOKEN_LEFT_SHIFT);

	LITERAL_TOKEN("++", TOKEN_DOUBLE_ADD);
	LITERAL_TOKEN("--", TOKEN_DOUBLE_SUB);
	LITERAL_TOKEN("+", TOKEN_ADD);
	LITERAL_TOKEN("-", TOKEN_SUB);
	LITERAL_TOKEN("*", TOKEN_MUL);
	LITERAL_TOKEN("/", TOKEN_DIV);
	LITERAL_TOKEN("%", TOKEN_PERCENT);

	LITERAL_TOKEN("==", TOKEN_DOUBLE_EQUAL);
	LITERAL_TOKEN("!=", TOKEN_NOT_EQUAL);
	LITERAL_TOKEN(">=", TOKEN_GREATER_EQUAL);
	LITERAL_TOKEN("<=", TOKEN_LESSER_EQUAL);
	LITERAL_TOKEN(">",  TOKEN_GREATER);
	LITERAL_TOKEN("<",  TOKEN_LESSER);

	LITERAL_TOKEN("!", TOKEN_EXCLAMATION);
	LITERAL_TOKEN("@", TOKEN_AT);
	LITERAL_TOKEN("#", TOKEN_HASH);
	LITERAL_TOKEN("=", TOKEN_EQUAL);
	LITERAL_TOKEN(":", TOKEN_COLON);
	LITERAL_TOKEN(";", TOKEN_STM_END);

    LITERAL_TOKEN(".", TOKEN_DOT);
    LITERAL_TOKEN(",", TOKEN_COMMA);
    LITERAL_TOKEN("(", TOKEN_PAR_OPEN);
    LITERAL_TOKEN(")", TOKEN_PAR_CLOSE);
    LITERAL_TOKEN("{", TOKEN_BRAC_OPEN);
    LITERAL_TOKEN("}", TOKEN_BRAC_CLOSE);
    LITERAL_TOKEN("[", TOKEN_SQ_BRAC_OPEN);
    LITERAL_TOKEN("]", TOKEN_SQ_BRAC_CLOSE);

	if (id()) {
		if (strcmp(nextText, "if") == 0) {
			this->nextType = TOKEN_IF;
			return true;
		} else if (strcmp(nextText, "else") == 0) {
			this->nextType = TOKEN_ELSE;
			return true;
		} else if (strcmp(nextText, "while") == 0) {
			this->nextType = TOKEN_WHILE;
			return true;
		} else if (strcmp(nextText, "break") == 0) {
			this->nextType = TOKEN_BREAK;
			return true;
		} else if (strcmp(nextText, "cast") == 0) {
			this->nextType = TOKEN_CAST;
			return true;
		} else if (strcmp(nextText, "struct") == 0) {
			this->nextType = TOKEN_STRUCT;
			return true;
		} else if (strcmp(nextText, "fn") == 0) {
			this->nextType = TOKEN_FUNCTION;
			return true;
		} else if (strcmp(nextText, "return") == 0) {
			this->nextType = TOKEN_RETURN;
			return true;
		} else if (strcmp(nextText, "import") == 0) {
			this->nextType = TOKEN_IMPORT;
			return true;
		} else return true;
	}

	FUNCTION_TOKEN(number);
	FUNCTION_TOKEN(string);

	fprintf(stderr, "Unrecognized token!\n");
	fprintf(stderr, " -> '%d'\n", this->buffer->peek());
	fprintf(stderr, "\tat %s:%d,%d\n", this->buffer->source,
		this->buffer->line, this->buffer->col);
	exit(EXIT_FAILURE);
	return false;
}

bool Lexer::is_next_type (Token_Type type) {
	return this->nextType == type;
}

void Lexer::skip (unsigned int count) {
	for (unsigned int i = 0; i < count; i++)
		this->parse_next();
}

char* Lexer::text () {
	char* text = this->nextText;
	this->skip(1);
	return text;
}

bool Lexer::check_skip (Token_Type type) {
	if (this->nextType == type) {
		this->skip(1);
		return true;
	} else {
		this->report_unexpected(type);
		return false;
	}
}

bool Lexer::optional_skip (Token_Type type) {
	if (this->nextType == type) {
		this->skip(1);
		return true;
	} else return false;
}

void Lexer::report_unexpected (Token_Type expected) {
	// TODO: make some sort of generic location info structure to pass
	Light_Compiler::inst->error(NULL, "Parser: Expected '%s', but got '%s'",
		token_get_text(expected), token_get_text(this->nextType));
	Light_Compiler::inst->error_stop(NULL, "@ %s:%d,%d", this->buffer->source,
		this->buffer->line, this->buffer->col);
}

void Lexer::report_unexpected () {
	Light_Compiler::inst->error(NULL, "Parser: Unexpected token '%s'",
		token_get_text(this->nextType));
	Light_Compiler::inst->error_stop(NULL, "@ %s:%d,%d", this->buffer->source,
		this->buffer->line, this->buffer->col);
	exit(EXIT_FAILURE);
}

#define LEXER_BUFFER_SIZE 128
char _buffer[LEXER_BUFFER_SIZE];
unsigned char _buffer_count;

bool Lexer::id () {
	char c = this->buffer->peek();
    if (ALPHA(c)) {
		this->nextType = TOKEN_ID;
		_buffer_count = 0;
        while (ALPHANUM(c)) {
			_buffer[_buffer_count++] = c;
			assert(_buffer_count < LEXER_BUFFER_SIZE);
			this->buffer->skip();
	        c = this->buffer->peek();
        }
		_buffer[_buffer_count] = 0;
		this->nextText = strdup(_buffer);
        return true;
    }
    return false;
}

bool Lexer::string () {
	char c = this->buffer->peek();
    if (c == '"') {
		this->buffer->skip();
        this->nextType = TOKEN_STRING;

		_buffer_count = 0;
		c = this->buffer->next();
		while (c != '"') {
			if (c == '\\') {
				c = this->buffer->next();
				if (c == 'n') {
					_buffer[_buffer_count++] = '\n';
					c = this->buffer->next();
					continue;
				} else if (c == 't') {
					_buffer[_buffer_count++] = '\t';
					c = this->buffer->next();
					continue;
				}
			}
			_buffer[_buffer_count++] = c;
			c = this->buffer->next();
		}
		_buffer[_buffer_count] = 0;
		this->nextText = strdup(_buffer);
        return true;
    } else return false;
}

bool Lexer::number () {
	_buffer_count = 0;
	char c = this->buffer->peek();
	if (c == '0' && this->buffer->peek(1) == 'x') {
		while (ALPHANUM(c) || c == 'x') {
			_buffer[_buffer_count++] = c;
			this->buffer->skip();
			c = this->buffer->peek();
		}
		this->nextType = TOKEN_NUMBER;
		_buffer[_buffer_count] = 0;
		this->nextText = strdup(_buffer);
		return true;
	} else {
	    if (c == '+' || c == '-') {
	        _buffer[_buffer_count++] = c;
			this->buffer->skip();
	        c = this->buffer->peek();
	    }
	    if (DIGIT(c) || c == '.') {
	        while (DIGIT(c)) {
				_buffer[_buffer_count++] = c;
				this->buffer->skip();
		        c = this->buffer->peek();
	        }
	        if (c == '.') {
	            _buffer[_buffer_count++] = c;
	            this->buffer->skip();
	            c = this->buffer->peek();
	            while (DIGIT(c)) {
		            _buffer[_buffer_count++] = c;
		            this->buffer->skip();
		            c = this->buffer->peek();
	            }
	        }
	        this->nextType = TOKEN_NUMBER;
			_buffer[_buffer_count] = 0;
			this->nextText = strdup(_buffer);
	        return true;
	    }
	}
    return false;
}

void Lexer::handleToken (Token_Type type, const char* text) {
	this->buffer->skip(strlen(text));
	this->nextText = NULL;
	this->nextType = type;
}

bool Lexer::skip_ignored_and_comments () {
	this->buffer->skipAny(LEXER_IGNORED);
    if (this->buffer->peek() == '/') {
        if (this->buffer->peek(1) == '/') {
			this->buffer->skipUntil("\n");
			this->buffer->skipAny(LEXER_IGNORED);
			return true;
        } else if (this->buffer->peek(1) == '*') {
			this->buffer->skipUntil("*/");
			this->buffer->skipAny(LEXER_IGNORED);
			return true;
        }
    }
	return false;
}

Lexer* Lexer::push (const char* filepath) {
	return new Lexer(filepath, this);
}

Lexer* Lexer::pop () {
	auto out = this->parent;
	delete this;
	return out;
}
