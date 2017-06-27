#ifndef LIGHT_LANG_LEXER_H
#define LIGHT_LANG_LEXER_H

#include <stdio.h>
#include <stdbool.h>

#include "buffer.c/buffer.h"

#define LEXER_DEBUG false

#define LEXER_IGNORED " \n\t"

#define LITERAL_TOKEN(lexer, string, type)                                  \
    if (buffer_is_next_str(lexer->buffer, string))                          \
        {_token(lexer, token, type, strlen(string)); return true;}

#define ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||       \
    c == '$' || c == '_')
#define DIGIT(c) (c >= '0' && c <= '9')
#define ALPHANUM(c) (ALPHA(c) || DIGIT(c))

typedef enum {
    ASSIGN,
    ADD,
    SUB,
    DIV,
    MUL,
    LET,
    TYPE,
    FUNCTION,
    PAR_OPEN,
    PAR_CLOSE,
    BRAC_OPEN,
    BRAC_CLOSE,
    SQ_BRAC_OPEN,
    SQ_BRAC_CLOSE,
    STM_END,
    COMMA,
    RETURN,

    ID,
    NUMBER,
} TokenType;

typedef struct {
    TokenType type;
    char* text;
    int line;
    int col;
} LexerToken;

typedef struct {
    Buffer* buffer;
    List* pushback;
} Lexer;

static inline
void _token (Lexer* lexer, LexerToken* token, TokenType type, int size);
static inline
void lexer_skip_ignored_and_comments (Lexer* lexer);

static inline bool lexer_id (Lexer* lexer, LexerToken* token);
static inline bool lexer_number (Lexer* lexer, LexerToken* token);

Lexer* lexer_create (char* filename) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->buffer = buffer_create(filename, "r");
    lexer->pushback = list_create(sizeof(LexerToken));
    return lexer;
}

void lexer_error (Lexer* lexer, const char* message) {
    printf("[Light] Error(%d, %d): %s\n",
        lexer->buffer->line,
        lexer->buffer->col, message);
    exit(1);
}

bool lexer_parse_next (Lexer* lexer, LexerToken* token) {
    if (LEXER_DEBUG) printf("[LEXER] lexer_parse_next (%d)\n", token);
    lexer_skip_ignored_and_comments(lexer);
    LITERAL_TOKEN(lexer, ";", STM_END);
    LITERAL_TOKEN(lexer, ",", COMMA);
    LITERAL_TOKEN(lexer, "(", PAR_OPEN);
    LITERAL_TOKEN(lexer, ")", PAR_CLOSE);
    LITERAL_TOKEN(lexer, "{", BRAC_OPEN);
    LITERAL_TOKEN(lexer, "}", BRAC_CLOSE);
    LITERAL_TOKEN(lexer, "[", SQ_BRAC_OPEN);
    LITERAL_TOKEN(lexer, "]", SQ_BRAC_CLOSE);
    LITERAL_TOKEN(lexer, "/", DIV);
    LITERAL_TOKEN(lexer, "*", MUL);
    LITERAL_TOKEN(lexer, "+", ADD);
    LITERAL_TOKEN(lexer, "-", SUB);
    LITERAL_TOKEN(lexer, "=", ASSIGN);
    LITERAL_TOKEN(lexer, "let", LET);
    LITERAL_TOKEN(lexer, "type", TYPE);
    LITERAL_TOKEN(lexer, "function", FUNCTION);
    LITERAL_TOKEN(lexer, "return", RETURN);
    if (lexer_number(lexer, token)) return true;
    if (lexer_id(lexer, token)) return true;
    return false;
}

bool lexer_hasNext (Lexer* lexer) {
    lexer_skip_ignored_and_comments(lexer);
    return buffer_hasNext(lexer->buffer);
}

bool lexer_next (Lexer* lexer, LexerToken* token) {
    if (LEXER_DEBUG) printf("[LEXER] lexer_next (%d)\n", token);
    bool found = false;
    if (lexer->pushback->size > 0) {
        list_pop(lexer->pushback, token);
        found = true;
    } else found = lexer_parse_next(lexer, token);
    return found;
}

char* lexer_next_text (Lexer* lexer) {
    static LexerToken token;
    lexer_next(lexer, &token);
    return token.text;
}

static inline
void fill_pushback (Lexer* lexer, int offset) {
    if (LEXER_DEBUG) printf("[LEXER] fill_pushback (%d)\n", offset);
    LexerToken token;
    for (int i = lexer->pushback->size; i < offset; i++) {
        if (!lexer_parse_next(lexer, &token)) break;
        list_add(lexer->pushback, &token, 0);
    }
}

TokenType lexer_peek (Lexer* lexer, int offset) {
    if (LEXER_DEBUG) printf("[LEXER] lexer_peek (%d)\n", offset);
    offset += 1;
    LexerToken token;
    fill_pushback(lexer, offset);
    list_get(lexer->pushback, &token, lexer->pushback->size - offset);
    return token.type;
}

bool lexer_is_next (Lexer* lexer, TokenType type) {
    if (LEXER_DEBUG) printf("[LEXER] lexer_is_next (%d)\n", type);
    return lexer_peek(lexer, 0) == type;
}

void lexer_skip (Lexer* lexer, int count) {
    LexerToken token;
    for (int i = 0; i < count; i++) lexer_next(lexer, &token);
}

void lexer_pushback (Lexer* lexer, LexerToken* token) {
    list_push(lexer->pushback, token);
}

void lexer_destroy (Lexer* lexer) {
    list_destroy(lexer->pushback);
    free(lexer);
}

static inline
void _token (Lexer* lexer, LexerToken* token, TokenType type, int size) {
    buffer_skip(lexer->buffer, size);
    token->type = type;
}

static inline
bool lexer_id (Lexer* lexer, LexerToken* token) {
    if (LEXER_DEBUG) printf("ID\n");
    bool found = false;
    List* _buff = list_create(sizeof(char));
    char c = buffer_peek(lexer->buffer, 0);
    if (ALPHA(c)) {
        while (ALPHANUM(c)) {
            list_push_char(_buff, c);
            buffer_skip(lexer->buffer, 1);
            c = buffer_peek(lexer->buffer, 0);
        }
        token->text = malloc(sizeof(char) * (_buff->size + 1));
        memcpy(token->text, _buff->data, _buff->size);
        token->text[_buff->size] = '\0';
        token->type = ID;
        found = true;
    }
    list_destroy(_buff);
    return found;
}

static inline
bool lexer_number (Lexer* lexer, LexerToken* token) {
    if (LEXER_DEBUG) printf("NUMBER\n");
    bool found = false;
    List* _buff = list_create(sizeof(char));
    char c = buffer_peek(lexer->buffer, 0);
    if (c == '+' || c == '-') {
        list_push_char(_buff, c);
        buffer_skip(lexer->buffer, 1);
        c = buffer_peek(lexer->buffer, 0);
    }
    if (DIGIT(c) || c == '.') {
        while (DIGIT(c)) {
            list_push_char(_buff, c);
            buffer_skip(lexer->buffer, 1);
            c = buffer_peek(lexer->buffer, 0);
        }
        if (c == '.') {
            list_push_char(_buff, c);
            buffer_skip(lexer->buffer, 1);
            c = buffer_peek(lexer->buffer, 0);
            while (DIGIT(c)) {
                list_push_char(_buff, c);
                buffer_skip(lexer->buffer, 1);
                c = buffer_peek(lexer->buffer, 0);
            }
        }
        token->text = malloc(sizeof(char) * (_buff->size + 1));
        memcpy(token->text, _buff->data, _buff->size);
        token->text[_buff->size] = '\0';
        token->type = NUMBER;
        found = true;
    }
    list_destroy(_buff);
    return found;
}

static inline
void lexer_skip_ignored_and_comments (Lexer* lexer) {
    buffer_skip_any(lexer->buffer, LEXER_IGNORED);
    if (buffer_peek(lexer->buffer, 0) == '/') {
        if (buffer_peek(lexer->buffer, 1) == '/') {
            buffer_skip_until(lexer->buffer, "\n");
            buffer_skip_any(lexer->buffer, LEXER_IGNORED);
        } else if (buffer_peek(lexer->buffer, 1) == '*') {
            buffer_skip_until(lexer->buffer, "*/");
            buffer_skip_any(lexer->buffer, LEXER_IGNORED);
        }
    }
}

#endif
