#pragma once

#include "module.hpp"
#include "scanner.hpp"
#include "token.hpp"
#include "compiler_events.hpp"
#include "util/events.hpp"

#include "phase/async_phase.hpp"

#include <vector>

#define LEXER_IGNORED " \n\t"

#define CASE_TOKEN(c, type) case c: { scanner->skip();							\
	return new Token(&scanner->location, type); }

#define MULTI_CASE_TOKEN(c, type, body) case c: { scanner->skip();				\
	switch (scanner->peek()) {													\
		body																	\
		default: return new Token(&scanner->location, type);					\
	}																			\
}

#define CHECK_DYN_TOKEN(func, type) tmp = func(scanner);						\
	if (tmp) return new Token(&scanner->location, type, scanner->ref() - tmp, tmp);

#define LEN(text) (sizeof(text) - 1)

#define CHECK_STR_TOKEN(text, type) if (scanner->is_next(text, LEN(text)))		\
	{ scanner->skip(LEN(text)); return new Token(&scanner->location, type); }

#define ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
#define DIGIT(c) (c >= '0' && c <= '9')
#define ALPHANUM(c) (ALPHA(c) || DIGIT(c))
#define SIGN(c) (c == '+' || c == '-')

struct Lexer : Async_Phase {
	uint64_t files_lexed = 0;
	uint64_t lines_of_code = 0;
	uint64_t token_count = 0;

	Lexer () : Async_Phase("Lexer", CE_MODULE_RUN_LEXER) { /* empty */ }

    void handle_main_event (void* data) {
		auto absolute_path = reinterpret_cast<char*>(data);

		auto module = new Module();
		module->absolute_path = absolute_path;

		auto scanner = Scanner(absolute_path);
		module->full_source = scanner.source;

		this->source_to_tokens(&scanner, &module->tokens);
		Events::trigger(this->event_to_id, module);
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

		switch (scanner->peek()) {
			CASE_TOKEN(';', TOKEN_STM_END)
			CASE_TOKEN('(', TOKEN_PAR_OPEN)
			CASE_TOKEN(')', TOKEN_PAR_CLOSE)
			CASE_TOKEN('+', TOKEN_ADD)
			CASE_TOKEN('*', TOKEN_MUL)
			CASE_TOKEN('/', TOKEN_DIV)
			CASE_TOKEN('%', TOKEN_PERCENT)
			CASE_TOKEN('^', TOKEN_CARET)
			CASE_TOKEN('~', TOKEN_TILDE)
			CASE_TOKEN('$', TOKEN_DOLLAR)
			CASE_TOKEN('@', TOKEN_AT)
			CASE_TOKEN('#', TOKEN_HASH)
			CASE_TOKEN(':', TOKEN_COLON)
			CASE_TOKEN('.', TOKEN_DOT)
			CASE_TOKEN(',', TOKEN_COMMA)
			CASE_TOKEN('{', TOKEN_BRAC_OPEN)
			CASE_TOKEN('}', TOKEN_BRAC_CLOSE)
			CASE_TOKEN('[', TOKEN_SQ_BRAC_OPEN)
			CASE_TOKEN(']', TOKEN_SQ_BRAC_CLOSE)
			MULTI_CASE_TOKEN('&', TOKEN_AMP,
				CASE_TOKEN('&', TOKEN_DOUBLE_AMP)
			)
			MULTI_CASE_TOKEN('|', TOKEN_PIPE,
				CASE_TOKEN('|', TOKEN_DOUBLE_PIPE)
			)
			MULTI_CASE_TOKEN('>', TOKEN_GREATER,
				CASE_TOKEN('>', TOKEN_RIGHT_SHIFT)
				CASE_TOKEN('=', TOKEN_GREATER_EQUAL)
			)
			MULTI_CASE_TOKEN('<', TOKEN_LESSER,
				CASE_TOKEN('<', TOKEN_LEFT_SHIFT)
				CASE_TOKEN('=', TOKEN_LESSER_EQUAL)
			)
			MULTI_CASE_TOKEN('!', TOKEN_EXCLAMATION,
				CASE_TOKEN('=', TOKEN_NOT_EQUAL)
			)
			MULTI_CASE_TOKEN('=', TOKEN_EQUAL,
				CASE_TOKEN('=', TOKEN_DOUBLE_EQUAL)
			)
			MULTI_CASE_TOKEN('-', TOKEN_SUB,
				CASE_TOKEN('>', TOKEN_ARROW)
			)
			default: {
				CHECK_STR_TOKEN("break", 	TOKEN_BREAK);
				CHECK_STR_TOKEN("cast", 	TOKEN_CAST);
				CHECK_STR_TOKEN("else", 	TOKEN_ELSE);
				CHECK_STR_TOKEN("fn", 		TOKEN_FUNCTION);
				CHECK_STR_TOKEN("false", 	TOKEN_FALSE);
				CHECK_STR_TOKEN("foreign", 	TOKEN_FOREIGN);
				CHECK_STR_TOKEN("if", 		TOKEN_IF);
				CHECK_STR_TOKEN("import", 	TOKEN_IMPORT);
				CHECK_STR_TOKEN("null", 	TOKEN_NULL);
				CHECK_STR_TOKEN("run", 		TOKEN_RUN);
				CHECK_STR_TOKEN("return", 	TOKEN_RETURN);
				CHECK_STR_TOKEN("struct", 	TOKEN_STRUCT);
				CHECK_STR_TOKEN("true", 	TOKEN_TRUE);
				CHECK_STR_TOKEN("while", 	TOKEN_WHILE);

				CHECK_DYN_TOKEN(this->identifier, TOKEN_ID);
				CHECK_DYN_TOKEN(this->string, TOKEN_STRING);
				CHECK_DYN_TOKEN(this->number, TOKEN_NUMBER);

				return new Token(&scanner->location, TOKEN_EOF);
			}
		}
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
