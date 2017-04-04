#include <stdio.h>
#include <stdbool.h>

#include "buffer.c/buffer.h"

#define LEXER_LEFT_SEPARATOR "->"
#define LEXER_IGNORED " \t\n"
#define LEXER_RULE_END ";"

typedef struct {
    List* leftName;
    List* rightNames;
} LexerRule;

typedef struct {
    List* grammarName;
    List* grammarVersion;
    List* ignored;
    List* rules;
} Lexer;

static void parseGrammar (Lexer* lexer, Buffer* buffer);
static void parseGrammarRule (Lexer* lexer, Buffer* buffer);

Lexer* lexer_create (const char* file) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->rules = list_create(sizeof(LexerRule));
    Buffer* buffer = buffer_create(file, "r");

    parseGrammar(lexer, buffer);

    buffer_destroy(buffer);
    return lexer;
}

static void parseGrammar (Lexer* lexer, Buffer* buffer) {
    buffer_skip_any(buffer, LEXER_IGNORED);
    char actual[7];
    if (!buffer_expect(buffer, "grammar", actual)) {
        printf("Error: grammar declaration must be the 1st thing!\n");
        printf("\tExample: 'grammar Demo 1.0.0'\n");
        exit(1);
    }
    buffer_skip_any(buffer, LEXER_IGNORED);
    lexer->grammarName = buffer_read_until(buffer, LEXER_IGNORED);
    buffer_skip_any(buffer, LEXER_IGNORED);
    lexer->grammarVersion = buffer_read_until(buffer, LEXER_IGNORED);
    printf("Grammar: %s v%s\n", lexer->grammarName->data,
        lexer->grammarVersion->data);
    buffer_skip_any(buffer, LEXER_IGNORED);
    while (buffer_hasNext(buffer)) {
        parseGrammarRule(lexer, buffer);
        buffer_skip_any(buffer, LEXER_IGNORED);
    }
}

static void parseGrammarRule (Lexer* lexer, Buffer* buffer) {
    LexerRule rule;
    rule.rightNames = list_create(sizeof(List*));
    rule.leftName = buffer_read_until(buffer, LEXER_IGNORED);

    char actualSeparator[2];
    buffer_skip_any(buffer, LEXER_IGNORED);
    if (!buffer_expect(buffer, LEXER_LEFT_SEPARATOR, actualSeparator)) {
        printf("Error: expected rule left separator: '%s'\n", LEXER_LEFT_SEPARATOR);
        exit(1);
    }
    buffer_skip_any(buffer, LEXER_IGNORED);

    List* nextToken = buffer_read_until(buffer, LEXER_IGNORED);
    buffer_skip_any(buffer, LEXER_IGNORED);
    while (strcmp(nextToken->data, LEXER_RULE_END) != 0) {
        list_push(rule.rightNames, &nextToken);

        nextToken = buffer_read_until(buffer, LEXER_IGNORED);
        buffer_skip_any(buffer, LEXER_IGNORED);
    }

    int i;
    printf("%s ->", rule.leftName->data);
    for (i = 0; i < rule.rightNames->size; i++) {
        list_pop(rule.rightNames, &nextToken);
        printf(" %s", nextToken->data);
    }
    printf(" ;\n");

    list_push(lexer->rules, &rule);
}

void lexer_destroy (Lexer* lexer) {
    free(lexer);
}
