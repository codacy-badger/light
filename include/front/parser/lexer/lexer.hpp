#pragma once

#include "scanner.hpp"
#include "token.hpp"

#include <vector>
#include <assert.h>

#define LEXER_IGNORED " \n\t"

#define ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
#define DIGIT(c) (c >= '0' && c <= '9')
#define ALPHANUM(c) (ALPHA(c) || DIGIT(c))
#define SIGN(c) (c == '+' || c == '-')

#define MAX_TOKEN_PEEK 1
#define DEFAULT_ABSOLUTE_PATH "(from text)"

struct Lexer {
	const char* source_path = NULL;
	Scanner scanner;

	Token token_buffer[MAX_TOKEN_PEEK + 1];
	size_t token_index = 0;

	void set_source (const char* text, size_t length, const char* path) {
		this->source_path = path ? path : DEFAULT_ABSOLUTE_PATH;
		this->scanner.set_source(text, length);

		this->token_index = 0;
		for (size_t i = 0; i <= MAX_TOKEN_PEEK; i++) {
			this->parse_next_token(&(this->token_buffer[i]));
		}
	}

	void parse_next_token (Token* token) {
		this->skip_ignored_and_comments();

		if (!this->scanner.has_next()) {
			token->type = TOKEN_EOF;
			return;
		}

		token->location.absolute_path = this->source_path;
		token->location.line = this->scanner.current_line;
		token->location.col_begin = this->scanner.current_col;

		switch (scanner.peek()) {
			case ';': { scanner.skip(); token->type = TOKEN_STM_END; break; }
			case '(': { scanner.skip(); token->type = TOKEN_PAR_OPEN; break; }
			case ')': { scanner.skip(); token->type = TOKEN_PAR_CLOSE; break; }
			case '+': { scanner.skip(); token->type = TOKEN_ADD; break; }
			case '*': { scanner.skip(); token->type = TOKEN_MUL; break; }
			case '/': { scanner.skip(); token->type = TOKEN_DIV; break; }
			case '%': { scanner.skip(); token->type = TOKEN_PERCENT; break; }
			case '^': { scanner.skip(); token->type = TOKEN_CARET; break; }
			case '~': { scanner.skip(); token->type = TOKEN_TILDE; break; }
			case '$': { scanner.skip(); token->type = TOKEN_DOLLAR; break; }
			case '@': { scanner.skip(); token->type = TOKEN_AT; break; }
			case '#': { scanner.skip(); token->type = TOKEN_HASH; break; }
			case ':': { scanner.skip(); token->type = TOKEN_COLON; break; }
			case '.': { scanner.skip(); token->type = TOKEN_DOT; break; }
			case ',': { scanner.skip(); token->type = TOKEN_COMMA; break; }
			case '{': { scanner.skip(); token->type = TOKEN_BRAC_OPEN; break; }
			case '}': { scanner.skip(); token->type = TOKEN_BRAC_CLOSE; break; }
			case '[': { scanner.skip(); token->type = TOKEN_SQ_BRAC_OPEN; break; }
			case ']': { scanner.skip(); token->type = TOKEN_SQ_BRAC_CLOSE; break; }
			case '&': {
				scanner.skip();
				switch (scanner.peek()) {
					case '&': { scanner.skip(); token->type = TOKEN_DOUBLE_AMP; break; }
					default: { token->type = TOKEN_AMP; break; }
				}
				break;
			}
			case '|': {
				scanner.skip();
				switch (scanner.peek()) {
					case '|': { scanner.skip(); token->type = TOKEN_DOUBLE_PIPE; break; }
					default: { token->type = TOKEN_PIPE; break; }
				}
				break;
			}
			case '>': {
				scanner.skip();
				switch (scanner.peek()) {
					case '>': { scanner.skip(); token->type = TOKEN_RIGHT_SHIFT; break; }
					case '=': { scanner.skip(); token->type = TOKEN_GREATER_EQUAL; break; }
					default: { token->type = TOKEN_GREATER; break; }
				}
				break;
			}
			case '<': {
				scanner.skip();
				switch (scanner.peek()) {
					case '<': { scanner.skip(); token->type = TOKEN_LEFT_SHIFT; break; }
					case '=': { scanner.skip(); token->type = TOKEN_LESSER_EQUAL; break; }
					default: { token->type = TOKEN_LESSER; break; }
				}
				break;
			}
			case '!': {
				scanner.skip();
				switch (scanner.peek()) {
					case '=': { scanner.skip(); token->type = TOKEN_NOT_EQUAL; break; }
					default: { token->type = TOKEN_EXCLAMATION; break; }
				}
				break;
			}
			case '=': {
				scanner.skip();
				switch (scanner.peek()) {
					case '=': { scanner.skip(); token->type = TOKEN_DOUBLE_EQUAL; break; }
					default: { token->type = TOKEN_EQUAL; break; }
				}
				break;
			}
			case '-': {
				scanner.skip();
				switch (scanner.peek()) {
					case '>': { scanner.skip(); token->type = TOKEN_ARROW; break; }
					default: { token->type = TOKEN_SUB; break; }
				}
				break;
			}
			default: {
				if (this->identifier(token)) {
					if (token->equal("break", 	5)) { token->type = TOKEN_BREAK; }
					if (token->equal("cast", 	4)) { token->type = TOKEN_CAST; }
					if (token->equal("else", 	4)) { token->type = TOKEN_ELSE; }
					if (token->equal("fn", 		2)) { token->type = TOKEN_FUNCTION; }
					if (token->equal("false", 	5)) { token->type = TOKEN_FALSE; }
					if (token->equal("foreign", 7)) { token->type = TOKEN_FOREIGN; }
					if (token->equal("if", 		2)) { token->type = TOKEN_IF; }
					if (token->equal("import", 	6)) { token->type = TOKEN_IMPORT; }
					if (token->equal("null", 	4)) { token->type = TOKEN_NULL; }
					if (token->equal("run", 	3)) { token->type = TOKEN_RUN; }
					if (token->equal("return", 	6)) { token->type = TOKEN_RETURN; }
					if (token->equal("struct", 	6)) { token->type = TOKEN_STRUCT; }
					if (token->equal("true", 	4)) { token->type = TOKEN_TRUE; }
					if (token->equal("while", 	5)) { token->type = TOKEN_WHILE; }
				} else if (this->string(token) || this->number(token)) {
					break;
				} else {
					token->type = TOKEN_EOF;
				}
			}
		}

		token->location.col_end = this->scanner.current_col;
	}

	bool identifier (Token* token) {
	    if (ALPHA(scanner.peek())) {
			token->type = TOKEN_ID;
			token->text = this->scanner.current_location_pointer();

			size_t initial = scanner.index;
			scanner.skip();

			auto c = scanner.peek();
			while (ALPHANUM(c)) {
				scanner.skip();
				c = scanner.peek();
			}

			token->length = scanner.index - initial;
			return true;
	    } else return false;
	}

	size_t string (Token* token) {
	    if (scanner.peek() == '"') {
			token->type = TOKEN_STRING;
			token->text = this->scanner.current_location_pointer();

			size_t initial = scanner.index;
			scanner.skip();

			auto c = scanner.next();
			while (c != '"') {
				// INFO: if we find a \" we don't have to stop
				if (c == '\\') c = scanner.next();

				c = scanner.next();
			}

			token->length = scanner.index - initial;
			return true;
	    } else return false;
	}

	size_t number (Token* token) {
		token->type = TOKEN_NUMBER;
		token->text = this->scanner.current_location_pointer();

		size_t initial = scanner.index;

		char c = scanner.peek();
		if (c == '0' && scanner.peek(1) == 'x') {
			c = scanner.skip_and_peek(2);
			while (ALPHANUM(c)) {
				c = scanner.skip_and_peek();
			}
		} else if (c == '0' && scanner.peek(1) == 'b') {
			c = scanner.skip_and_peek(2);
			while (DIGIT(c)) {
				c = scanner.skip_and_peek();
			}
		} else {
		    if (SIGN(c)) {
		        c = scanner.skip_and_peek();
		    }
		    if (DIGIT(c) || c == '.') {
				while (DIGIT(c)) {
					c = scanner.skip_and_peek();
				}
		        if (c == '.') {
		            c = scanner.skip_and_peek();
					while (DIGIT(c)) {
						c = scanner.skip_and_peek();
					}
		        }
		    }
		}

		token->length = scanner.index - initial;
	    return token->length > 0;
	}

	void skip_ignored_and_comments () {
		while (true) {
			this->scanner.skip_any(LEXER_IGNORED);
		    if (this->scanner.peek() == '/') {
		        if (this->scanner.peek(1) == '/') {
					this->scanner.skip_until("\n");
					this->scanner.skip_any(LEXER_IGNORED);
					continue;
		        } else if (this->scanner.peek(1) == '*') {
					this->scanner.skip_until("*/");
					this->scanner.skip_any(LEXER_IGNORED);
					continue;
		        }
		    }
			return;
		}
	}

	const char* escape_string (const char* original, size_t length) {
		auto output = (char*) malloc(length - 1);

		size_t output_count = 0, i = 1;
		while (i < (length - 1)) {
			char c = original[i++];
			if (c == '\\') {
				c = original[i++];

				switch (c) {
					case 'n':	output[output_count++] = '\n'; break;
					case 't':	output[output_count++] = '\t'; break;
					default:	output[output_count++] = c;
				}
			} else output[output_count++] = c;
		}
		output[output_count] = '\0';

		return output;
	}

	const char* escaped_string () {
		auto token = this->peek();
		auto output = this->escape_string(token->text, token->length);
		this->skip();
		return output;
	}

	Token* peek (size_t offset = 0) {
		assert(offset <= MAX_TOKEN_PEEK);

		auto _index = this->token_index + offset;
		if (_index > MAX_TOKEN_PEEK) {
			_index -= (MAX_TOKEN_PEEK + 1);
		}

		return &(this->token_buffer[_index]);
	}

	void skip () {
		this->parse_next_token(&(this->token_buffer[this->token_index]));

		this->token_index += 1;
		if (this->token_index > MAX_TOKEN_PEEK) {
			this->token_index -= (MAX_TOKEN_PEEK + 1);
		}
	}

	bool is_next (Token_Type type) {
		return this->peek()->type == type;
	}

	bool try_skip (Token_Type type) {
		if (this->is_next(type)) {
			this->skip();
			return true;
		} else return false;
	}

	void expect (Token_Type type) {
		if (!this->try_skip(type)) {
			this->report_expected(Token::to_string(type));
		}
	}

	void report_expected (const char* c) {
		auto token = this->peek();
		printf("[ERROR] Expected character '%s'\n", c);
		printf("\t at '%s':(%zd,%zd)\n", this->source_path,
			token->location.line, token->location.col_begin);
	}
};
