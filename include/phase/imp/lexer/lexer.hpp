#pragma once

#include "module.hpp"
#include "scanner.hpp"
#include "token.hpp"
#include "compiler_events.hpp"
#include "events.hpp"

#include "phase/async_phase.hpp"

#include <vector>

#define LEXER_IGNORED " \n\t"

#define CHECK_STR_TOKEN(text, type, length) if (scanner->is_next(text, length))	\
	{ scanner->skip(length); return new Token(&scanner->location, type); }

#define CHECK_STR2_TOKEN(text, type) CHECK_STR_TOKEN(text, type, 2)

#define CHECK_CHAR_TOKEN(c, type) if (scanner->is_next(c))						\
	{ scanner->skip(); return new Token(&scanner->location, type); }

#define CHECK_DYN_TOKEN(func, type) tmp = func(scanner);						\
	if (tmp) return new Token(&scanner->location, type, scanner->ref() - tmp, tmp);

#define ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
#define DIGIT(c) (c >= '0' && c <= '9')
#define ALPHANUM(c) (ALPHA(c) || DIGIT(c))
#define SIGN(c) (c == '+' || c == '-')

struct Lexer : Async_Phase {
	uint64_t files_lexed = 0;
	uint64_t lines_of_code = 0;
	uint64_t token_count = 0;

	Lexer () : Async_Phase("Lexer", CE_MODULE_RUN_LEXER) {}

    void on_event (void* data) {
		auto absolute_path = reinterpret_cast<char*>(data);

		auto module = new Module();
		module->absolute_path = absolute_path;

		auto scanner = new Scanner(absolute_path);
		module->tokens = new std::vector<Token*>();

		this->source_to_tokens(scanner, module->tokens);
		Events::trigger(CE_MODULE_RUN_PARSER, module);

		delete scanner;
    }

	void source_to_tokens (Scanner* scanner, std::vector<Token*>* tokens) {
		Token* token = NULL;
		do {
			token = this->get_next_token(scanner);
			tokens->push_back(token);
		} while (token->type != TOKEN_EOF);

		this->lines_of_code += scanner->location.line;
		this->token_count += tokens->size();
		this->files_lexed += 1;
	}

	Token* get_next_token (Scanner* scanner) {
		size_t tmp;

		this->skip_ignored_and_comments(scanner);

		CHECK_STR_TOKEN("if", 		TOKEN_IF, 		2);
		CHECK_STR_TOKEN("else", 	TOKEN_ELSE, 	4);
		CHECK_STR_TOKEN("while", 	TOKEN_WHILE, 	5);
		CHECK_STR_TOKEN("break", 	TOKEN_BREAK, 	5);
		CHECK_STR_TOKEN("cast", 	TOKEN_CAST, 	4);
		CHECK_STR_TOKEN("struct", 	TOKEN_STRUCT, 	6);
		CHECK_STR_TOKEN("fn", 		TOKEN_FUNCTION, 2);
		CHECK_STR_TOKEN("return", 	TOKEN_RETURN, 	6);
		CHECK_STR_TOKEN("import", 	TOKEN_IMPORT, 	6);
		CHECK_STR_TOKEN("include", 	TOKEN_INCLUDE, 	7);
		CHECK_STR_TOKEN("foreign", 	TOKEN_FOREIGN, 	7);
		CHECK_STR_TOKEN("run", 		TOKEN_RUN, 		3);
		CHECK_STR_TOKEN("false", 	TOKEN_FALSE, 	5);
		CHECK_STR_TOKEN("true", 	TOKEN_TRUE, 	4);
		CHECK_STR_TOKEN("null", 	TOKEN_NULL, 	4);

		CHECK_DYN_TOKEN(this->identifier, TOKEN_ID);

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

		CHECK_CHAR_TOKEN(';', TOKEN_STM_END);
	    CHECK_CHAR_TOKEN('(', TOKEN_PAR_OPEN);
	    CHECK_CHAR_TOKEN(')', TOKEN_PAR_CLOSE);
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
	    CHECK_CHAR_TOKEN('.', TOKEN_DOT);
	    CHECK_CHAR_TOKEN(',', TOKEN_COMMA);
	    CHECK_CHAR_TOKEN('{', TOKEN_BRAC_OPEN);
	    CHECK_CHAR_TOKEN('}', TOKEN_BRAC_CLOSE);
	    CHECK_CHAR_TOKEN('[', TOKEN_SQ_BRAC_OPEN);
	    CHECK_CHAR_TOKEN(']', TOKEN_SQ_BRAC_CLOSE);

		CHECK_DYN_TOKEN(this->string, TOKEN_STRING);
		CHECK_DYN_TOKEN(this->number, TOKEN_NUMBER);

		return new Token(&scanner->location, TOKEN_EOF);
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
		    if (SIGN(c)) {
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

	void print_extra_metrics () {
		print_extra_metric("Files lexed", "%zd", this->files_lexed);
		print_extra_metric("Lines of Code", "%zd", this->lines_of_code);
		print_extra_metric("Tokens created", "%zd", this->token_count);
	}
};
