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

void lexer_printf(Lexer* lexer);

static void parseGrammar (Lexer* lexer, Buffer* buffer);
static void parseGrammarRule (Lexer* lexer, Buffer* buffer);
static void isolateLiterals (Lexer* lexer);
static void splitOptions (Lexer* lexer);

Lexer* lexer_create (const char* file) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->rules = list_create(sizeof(LexerRule));
    Buffer* buffer = buffer_create(file, "r");

    parseGrammar(lexer, buffer);
    lexer_printf(lexer);
    printf("\n");

    isolateLiterals(lexer);
    lexer_printf(lexer);
    printf("\n");

    splitOptions(lexer);
    lexer_printf(lexer);
    printf("\n");

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
    printf("Grammar %s v%s\n\n", lexer->grammarName->data,
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

    list_push(lexer->rules, &rule);
}

static void isolateLiterals (Lexer* lexer) {
    printf("Isolating literals...\n");
    int j, i;
    LexerRule rule;
    List* token;
    for (j = 0; j < lexer->rules->size; j++) {
        list_get(lexer->rules, &rule, j);
        for (i = 0; i < rule.rightNames->size; i++) {
            list_get(rule.rightNames, &token, i);
            if (rule.rightNames->size > 1 && token->data[0] == '\'') {
                List* newToken = list_copy(token);
                list_set_char(newToken, '$', 0);
                list_pop(newToken, NULL);
                list_set_char(newToken, '\0', newToken->size - 1);

                LexerRule newRule;
                newRule.leftName = newToken;
                newRule.rightNames = list_create(sizeof(List*));
                list_push(newRule.rightNames, &token);
                list_set(rule.rightNames, &newToken, i);
                list_push(lexer->rules, &newRule);
            }
        }
    }
}

static void splitOptions (Lexer* lexer) {
    printf("Splitting options...\n");
    int j, i, k;
    LexerRule rule;
    List* token;
    for (j = 0; j < lexer->rules->size; j++) {
        list_get(lexer->rules, &rule, j);
        for (i = 0; i < rule.rightNames->size; i++) {
            list_get(rule.rightNames, &token, i);
            if (strcmp(token->data, "|") == 0) {
                list_remove(rule.rightNames, NULL, i);

                LexerRule newRule;
                newRule.leftName = rule.leftName;
                newRule.rightNames = list_create(sizeof(List*));
                for (k = i; k < rule.rightNames->size;) {
                    list_remove(rule.rightNames, &token, k);
                    list_push(newRule.rightNames, &token);
                }
                list_push(lexer->rules, &newRule);
            }
        }
    }
}

void lexer_printf(Lexer* lexer) {
    int j, i;
    LexerRule rule;
    List* nextToken;
    for (j = 0; j < lexer->rules->size; j++) {
        list_get(lexer->rules, &rule, j);
        printf("%s ->", rule.leftName->data);
        for (i = 0; i < rule.rightNames->size; i++) {
            list_get(rule.rightNames, &nextToken, i);
            printf(" %s", nextToken->data);
        }
        printf(" ;\n");
    }
}

void lexer_destroy (Lexer* lexer) {
    //TODO: do it right...
    free(lexer);
}
