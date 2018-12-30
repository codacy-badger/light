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

#define CONSUME(check, c) while (check(c)) { _buffer[count++] = c; scanner->skip(); c = scanner->peek(); }

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

		auto tmp = this->identifier(scanner);
		if (tmp) {
			return new Token(&scanner->location, TOKEN_ID, scanner->ref() - tmp, tmp);
		}

		tmp = this->string(scanner);
		if (tmp) {
			return new Token(&scanner->location, TOKEN_STRING, scanner->ref() - tmp, tmp);
		}

		tmp = this->number(scanner);
		if (tmp) {
			return new Token(&scanner->location, TOKEN_NUMBER, scanner->ref() - tmp, tmp);
		}

		return NULL;
	}

	size_t identifier (Scanner* scanner) {
	    if (ALPHA(scanner->peek())) {
			size_t initial = scanner->index;
			scanner->skip();

			auto c = scanner->peek();
			while (ALPHANUM(c)) {
				scanner->skip();

				c = scanner->peek();
			}

			return scanner->index - initial;
	    } else return 0;
	}

	size_t string (Scanner* scanner) {
	    if (scanner->peek() == '"') {
			size_t initial = scanner->index;
			scanner->skip();

			auto c = scanner->next();
			while (c != '"') {
				// INFO: if we find a \" we don't have to stop
				if (c == '\\') c = scanner->next();

				c = scanner->next();
			}

			return scanner->index - initial;
	    } else return 0;
	}

	size_t number (Scanner* scanner) {
		size_t initial = scanner->index;

		char c = scanner->peek();
		if (c == '0' && scanner->peek(1) == 'x') {
			c = scanner->skip_and_peek(2);
			while (ALPHANUM(c)) {
				c = scanner->skip_and_peek();
			}
		} else if (c == '0' && scanner->peek(1) == 'b') {
			c = scanner->skip_and_peek(2);
			while (DIGIT(c)) {
				c = scanner->skip_and_peek();
			}
		} else {
		    if (c == '+' || c == '-') {
		        c = scanner->skip_and_peek();
		    }
		    if (DIGIT(c) || c == '.') {
				while (DIGIT(c)) {
					c = scanner->skip_and_peek();
				}
		        if (c == '.') {
		            c = scanner->skip_and_peek();
					while (DIGIT(c)) {
						c = scanner->skip_and_peek();
					}
		        }
		    }
		}

	    return scanner->index - initial;
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
