#ifndef LIGHT_LANG_PARSER_H
#define LIGHT_LANG_PARSER_H

#include "buffer.c/buffer.h"

#include "lexer.h"
#include "parser_ast_nodes.h"

/* MACROS - BEGIN */
#define PARSER_FUNCTION(node) PARSER_FUNCTION_TYPE(node, ast_##node)
#define PARSER_FUNCTION_TYPE(node, type) type* parse_##node (Lexer* lexer)
#define PARSE(node) parse_##node (lexer)

#define CHECK_AST(var, text, parent)											\
	if ((var) == NULL) { lexer_error(lexer, text); free(parent); return NULL; }

#define CHECK_TOKEN(var, text, parent)											\
	if (!(var)) { lexer_error(lexer, text); free(parent); return NULL; }

#define PARSE_AST(name, message, parent)										\
	ast_##name* _##name = parse_##name(lexer);									\
	CHECK_AST(_##name, (message), (parent))										\
	parent->name = _##name;

#define PARSE_TOKEN(token, message, parent)										\
	CHECK_TOKEN(lexer_is_next(token), ##message, ##parent)						\
	lexer_skip(lexer, 1);
/* MACROS - END */

PARSER_FUNCTION(expression);
PARSER_FUNCTION(statement);

PARSER_FUNCTION(type) {
    if (lexer_is_next(lexer, ID)) {
        ast_type* _type = malloc(sizeof(ast_type));
        _type->name = lexer_next_text(lexer);;
        return _type;
    } else return NULL;
}

PARSER_FUNCTION(variable) {
    if (lexer_is_next(lexer, ID)) {
        ast_variable* _var = malloc(sizeof(ast_variable));
        _var->exp.type = EXP_VARIABLE;
        _var->name = lexer_next_text(lexer);
        return _var;
    } else return NULL;
}

// statements

PARSER_FUNCTION(var_definition) {
	bool hasLet = lexer_is_next(lexer, LET);
	ast_type* _type = PARSE(type);
	if (!hasLet && _type == NULL) return NULL;

    ast_var_definition* _var_definition = calloc(1, sizeof(ast_var_definition));
	if (_type != NULL) _var_definition->type = _type;
	else lexer_skip(lexer, 1);
    _var_definition->stm.type = STM_VARDEF;

    if (lexer_is_next(lexer, ID)) {
        _var_definition->name = lexer_next_text(lexer);
    } else lexer_error(lexer, "Expected Identifier in variable definition");

    if (lexer_is_next(lexer, ASSIGN)) {
        lexer_skip(lexer, 1);
		PARSE_AST(expression, "Expected expression after equal in variable definition", _var_definition)
    }

	if (lexer_is_next(lexer, STM_END)) lexer_skip(lexer, 1);
	else lexer_error(lexer, "Expected ';' after statement");

    return _var_definition;
}

PARSER_FUNCTION(statement_group) {
	if (!lexer_is_next(lexer, BRAC_OPEN)) return NULL;
	else lexer_skip(lexer, 1);

    ast_statement* stm = PARSE(statement);
    if (stm == NULL) {
		lexer_error(lexer, "Expected statement after '{'");
		return NULL;
	}

    List* stms = list_create(sizeof(ast_statement*));
    while (stm != NULL) {
        list_push(stms, &stm);
        stm = PARSE(statement);
    }

    if (!lexer_is_next(lexer, BRAC_CLOSE)) {
		lexer_error(lexer, "Expected '}' after statements");
		return NULL;
	}

	ast_statement_group* _stm_group = NULL;
    _stm_group = malloc(sizeof(ast_statement_group));
    _stm_group->stm.type = STM_GROUP;
    _stm_group->stms = list_copyBytes(stms);
    _stm_group->stmsCount = stms->size;
	list_destroy(stms);
    return _stm_group;
}

PARSER_FUNCTION(statement) {
	ast_statement* stmt = NULL;

	stmt = (ast_statement*) PARSE(statement_group);
	if (stmt != NULL) return stmt;

	stmt = (ast_statement*) PARSE(var_definition);
	if (stmt != NULL) return stmt;

	return NULL;
}

// expressions

PARSER_FUNCTION_TYPE(factor, ast_expression) {
    if (lexer_is_next(lexer, PAR_OPEN)) {
        lexer_skip(lexer, 1);
        void* exp = PARSE(expression);
        if (lexer_is_next(lexer, PAR_CLOSE)) lexer_skip(lexer, 1);
        else lexer_error(lexer, "Expected close parenthesys!");
        return exp;
    } else if (lexer_is_next(lexer, SUB)) {
        ast_expression_unop* unop = malloc(sizeof(ast_expression_unop));
        unop->exp.type = EXP_UNOP;
        unop->type = EXP_UNOP_NEG;
        lexer_skip(lexer, 1);
        unop->child = PARSE(factor);
        return (ast_expression*)unop;
    } else if (lexer_is_next(lexer, ADD)) {
        lexer_skip(lexer, 1);
        return PARSE(expression);
    } else if (lexer_is_next(lexer, NUMBER)) {
        ast_expression_number* _number = malloc(sizeof(ast_expression_number));
        _number->exp.type = EXP_NUMBER;
        _number->value = lexer_next_text(lexer);
        return (ast_expression*)_number;
    } else if (lexer_is_next(lexer, ID)) {
        return (ast_expression*)PARSE(variable);
    } else return NULL;
}

PARSER_FUNCTION_TYPE(term, ast_expression) {
    ast_expression* lhs = PARSE(factor);
    if (lhs != NULL) {
        TokenType tt = lexer_peek(lexer, 0);
        while (tt == MUL || tt == DIV) {
            ast_expression_binop* binop = malloc(sizeof(ast_expression_binop));
            binop->exp.type = EXP_BINOP;
            binop->lhs = lhs;
            if (tt == MUL) {
                binop->type = EXP_BINOP_MUL;
            } else {
                binop->type = EXP_BINOP_DIV;
            }
            lexer_skip(lexer, 1);
            binop->rhs = PARSE(factor);
            if (binop->rhs == NULL) lexer_error(lexer, "Expected algebraic expression!");
            tt = lexer_peek(lexer, 0);
            lhs = (ast_expression*)binop;
        }
        return lhs;
    } else return NULL;
}

PARSER_FUNCTION(expression) {
    ast_expression* lhs = PARSE(term);
    if (lhs != NULL) {
        TokenType tt = lexer_peek(lexer, 0);
        while (tt == ADD || tt == SUB) {
            ast_expression_binop* binop = malloc(sizeof(ast_expression_binop));
            binop->exp.type = EXP_BINOP;
            binop->lhs = lhs;
            if (tt == ADD) {
                binop->type = EXP_BINOP_ADD;
            } else {
                binop->type = EXP_BINOP_SUB;
            }
            lexer_skip(lexer, 1);
            binop->rhs = PARSE(term);
            if (binop->rhs == NULL) lexer_error(lexer, "Expected algebraic expression!");
            tt = lexer_peek(lexer, 0);
            lhs = (ast_expression*)binop;
        }
        return lhs;
    } else return NULL;
}

#endif
