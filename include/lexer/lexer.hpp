#pragma once

#include "scanner.hpp"
#include "token.hpp"

#include <vector>

#define LEXER_IGNORED " \n\t"
#define LEXER_BUFFER_SIZE 256

#define CHECK_STR_TOKEN(text, type) if (scanner->is_next(text))					\
	{ scanner->skip(strlen(text)); return new Token(&scanner->location, type); }

#define CHECK_STR2_TOKEN(text, type) if (scanner->is_next(text))				\
	{ scanner->skip(2); return new Token(&scanner->location, type); }

#define CHECK_CHAR_TOKEN(c, type) if (scanner->is_next(c))						\
	{ scanner->skip(); return new Token(&scanner->location, type); }

#define CHECK_DYN_TOKEN(func, type) auto tmp_##func = func(scanner);			\
	if (tmp_##func) return new Token(&scanner->location, type, tmp_##func);

#define ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
#define DIGIT(c) (c >= '0' && c <= '9')
#define ALPHANUM(c) (ALPHA(c) || DIGIT(c))

#define _CONSUME(check, c) while (check(c)) { _buffer[count++] = c;				\
	scanner->skip(); c = scanner->peek(); }										\
	ASSERT(count < LEXER_BUFFER_SIZE);

struct Lexer {
	uint64_t lines_of_code = 0;
	uint64_t token_count = 0;
	double total_time = 0;

	void source_to_tokens (Scanner* scanner, std::vector<Token*>* tokens) {
		auto start = os_get_user_time();

		ASSERT(tokens->empty());
		auto token = this->get_next_token(scanner);
		while (token != NULL) {
			tokens->push_back(token);
			token = this->get_next_token(scanner);
		}

		this->lines_of_code += scanner->location.line;
		this->token_count += tokens->size();
		this->total_time += os_time_user_stop(start);
	}

	Token* get_next_token (Scanner* scanner) {
		this->skip_ignored_and_comments(scanner);

		CHECK_STR2_TOKEN("->", TOKEN_ARROW);
		CHECK_STR2_TOKEN("&&", TOKEN_DOUBLE_AMP);
		CHECK_STR2_TOKEN("||", TOKEN_DOUBLE_PIPE);
		CHECK_STR2_TOKEN(">>", TOKEN_RIGHT_SHIFT);
		CHECK_STR2_TOKEN("<<", TOKEN_LEFT_SHIFT);
		CHECK_STR2_TOKEN("==", TOKEN_DOUBLE_EQUAL);
		CHECK_STR2_TOKEN("!=", TOKEN_NOT_EQUAL);
		CHECK_STR2_TOKEN(">=", TOKEN_GREATER_EQUAL);
		CHECK_STR2_TOKEN("<=", TOKEN_LESSER_EQUAL);
		CHECK_STR2_TOKEN("++", TOKEN_DOUBLE_ADD);
		CHECK_STR2_TOKEN("--", TOKEN_DOUBLE_SUB);
		CHECK_STR2_TOKEN("..", TOKEN_DOUBLE_DOT);

		CHECK_CHAR_TOKEN('+', TOKEN_ADD);
		CHECK_CHAR_TOKEN('-', TOKEN_SUB);
		CHECK_CHAR_TOKEN('*', TOKEN_MUL);
		CHECK_CHAR_TOKEN('/', TOKEN_DIV);
		CHECK_CHAR_TOKEN('%', TOKEN_PERCENT);
		CHECK_CHAR_TOKEN('>', TOKEN_GREATER);
		CHECK_CHAR_TOKEN('<', TOKEN_LESSER);
		CHECK_CHAR_TOKEN('&', TOKEN_AMP);
		CHECK_CHAR_TOKEN('|', TOKEN_PIPE);
		CHECK_CHAR_TOKEN('^', TOKEN_CARET);
		CHECK_CHAR_TOKEN('~', TOKEN_TILDE);
		CHECK_CHAR_TOKEN('!', TOKEN_EXCLAMATION);
		CHECK_CHAR_TOKEN('$', TOKEN_DOLLAR);
		CHECK_CHAR_TOKEN('@', TOKEN_AT);
		CHECK_CHAR_TOKEN('#', TOKEN_HASH);
		CHECK_CHAR_TOKEN('=', TOKEN_EQUAL);
		CHECK_CHAR_TOKEN(':', TOKEN_COLON);
		CHECK_CHAR_TOKEN(';', TOKEN_STM_END);
	    CHECK_CHAR_TOKEN('.', TOKEN_DOT);
	    CHECK_CHAR_TOKEN(',', TOKEN_COMMA);
	    CHECK_CHAR_TOKEN('(', TOKEN_PAR_OPEN);
	    CHECK_CHAR_TOKEN(')', TOKEN_PAR_CLOSE);
	    CHECK_CHAR_TOKEN('{', TOKEN_BRAC_OPEN);
	    CHECK_CHAR_TOKEN('}', TOKEN_BRAC_CLOSE);
	    CHECK_CHAR_TOKEN('[', TOKEN_SQ_BRAC_OPEN);
	    CHECK_CHAR_TOKEN(']', TOKEN_SQ_BRAC_CLOSE);

		CHECK_STR_TOKEN("if", TOKEN_IF);
		CHECK_STR_TOKEN("else", TOKEN_ELSE);
		CHECK_STR_TOKEN("while", TOKEN_WHILE);
		CHECK_STR_TOKEN("break", TOKEN_BREAK);
		CHECK_STR_TOKEN("cast", TOKEN_CAST);
		CHECK_STR_TOKEN("struct", TOKEN_STRUCT);
		CHECK_STR_TOKEN("fn", TOKEN_FUNCTION);
		CHECK_STR_TOKEN("return", TOKEN_RETURN);
		CHECK_STR_TOKEN("import", TOKEN_IMPORT);
		CHECK_STR_TOKEN("include", TOKEN_INCLUDE);
		CHECK_STR_TOKEN("foreign", TOKEN_FOREIGN);
		CHECK_STR_TOKEN("run", TOKEN_RUN);
		CHECK_STR_TOKEN("false", TOKEN_FALSE);
		CHECK_STR_TOKEN("true", TOKEN_TRUE);
		CHECK_STR_TOKEN("null", TOKEN_NULL);

		CHECK_DYN_TOKEN(identifier, TOKEN_ID);
		CHECK_DYN_TOKEN(string, TOKEN_STRING);
		CHECK_DYN_TOKEN(number, TOKEN_NUMBER);

		return NULL;
	}

	char* identifier (Scanner* scanner) {
		char c = scanner->peek();
	    if (ALPHA(c)) {
			size_t count = 0;
			char _buffer[LEXER_BUFFER_SIZE];

			while (ALPHANUM(c)) {
				_buffer[count++] = c;
				scanner->skip();
				c = scanner->peek();
			}
			ASSERT(count < LEXER_BUFFER_SIZE);

			_buffer[count] = 0;
			return _strdup(_buffer);
	    }
	    return NULL;
	}

	char* string (Scanner* scanner) {
		char c = scanner->peek();
	    if (c == '"') {
			scanner->skip();
			size_t count = 0;
			char _buffer[LEXER_BUFFER_SIZE];

			c = scanner->next();
			while (c != '"') {
				ASSERT(count < LEXER_BUFFER_SIZE);
				if (c == '\\') {
					c = scanner->next();
					switch (c) {
						case 'n': _buffer[count++] = '\n'; break;
						case 't': _buffer[count++] = '\t'; break;
						default:  _buffer[count++] = c;    break;
					}
				} else _buffer[count++] = c;
				c = scanner->next();
			}
			_buffer[count] = 0;

			for (size_t i = 0; i < count; i++) {
				if (_buffer[i] == '\\') {
					switch (_buffer[i + 1]) {
						case 'n': _buffer[i] = '\n'; 			break;
						case 't': _buffer[i] = '\t'; 			break;
						default:  _buffer[i] = _buffer[i + 1];	break;
					}

					memcpy(&_buffer[i + 1], &_buffer[i + 2], count);
					i += 1;
				}
			}

			return _strdup(_buffer);
	    } else return NULL;
	}

	char* number (Scanner* scanner) {
		size_t count = 0;
		char _buffer[LEXER_BUFFER_SIZE];

		char c = scanner->peek();
		if (c == '0' && scanner->peek(1) == 'x') {
			_buffer[count++] = '0';
			_buffer[count++] = 'x';
			scanner->skip(2);
			c = scanner->peek();
			_CONSUME(ALPHANUM, c)
			_buffer[count] = 0;

			return _strdup(_buffer);
		} else if (c == '0' && scanner->peek(1) == 'b') {
			_buffer[count++] = '0';
			_buffer[count++] = 'b';
			scanner->skip(2);
			c = scanner->peek();
			_CONSUME(DIGIT, c)
			_buffer[count] = 0;

			return _strdup(_buffer);
		} else {
		    if (c == '+' || c == '-') {
		        _buffer[count++] = c;
				scanner->skip();
		        c = scanner->peek();
		    }
		    if (DIGIT(c) || c == '.') {
				_CONSUME(DIGIT, c)
		        if (c == '.') {
		            _buffer[count++] = c;
		            scanner->skip();
		            c = scanner->peek();
					_CONSUME(DIGIT, c)
		        }
				_buffer[count] = 0;

				return _strdup(_buffer);
		    }
		}
	    return NULL;
	}

	void skip_ignored_and_comments (Scanner* scanner) {
		while (true) {
			scanner->skip_any(LEXER_IGNORED);
		    if (scanner->peek() == '/') {
		        if (scanner->peek(1) == '/') {
					scanner->skip_until("\n");
					scanner->skip_any(LEXER_IGNORED);
					continue;
		        } else if (scanner->peek(1) == '*') {
					scanner->skip_until("*/");
					scanner->skip_any(LEXER_IGNORED);
					continue;
		        }
		    }
			return;
		}
	}

	void print_metrics (double userInterval) {
		double percent = (this->total_time * 100.0) / userInterval;
		printf("  - %-25s%8.6fs (%5.2f%%)\n", "Lexer", this->total_time, percent);
		printf("        Lines of Code:         %zd\n", this->lines_of_code);
		printf("        Tokens created:        %zd\n", this->token_count);
	}
};
