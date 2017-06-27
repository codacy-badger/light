#include <stdio.h>

#include "../parser.h"

#define TABS(tabs) for (int i = 0; i < tabs; i++) printf("    ");

void print_expression (ast_expression* expression, int tabs);

void print_expression_binop (ast_expression_binop* exp_binop, int tabs) {
    printf("(");
    switch (exp_binop->type) {
        case EXP_BINOP_ADD: printf("+"); break;
        case EXP_BINOP_SUB: printf("-"); break;
        case EXP_BINOP_MUL: printf("*"); break;
        case EXP_BINOP_DIV: printf("/"); break;
    }
    printf(" ");
    print_expression(exp_binop->lhs, tabs);
    printf(" ");
    print_expression(exp_binop->rhs, tabs);
    printf(")");
}

void print_var_definition (ast_var_definition* vardefStm, int tabs) {
    if (vardefStm != NULL) {
        TABS(tabs);
        printf("'%s' as '%s' --> ", vardefStm->name,
            vardefStm->type == NULL ? "?" : vardefStm->type->name);
        if (vardefStm->expression != NULL) {
            printf("\n");
            TABS(tabs + 1);
            print_expression(vardefStm->expression, tabs + 1);
        } else printf("?");
        printf("\n");
    }
}

void print_paramsdef (ast_params_def* params, int tabs) {
    if (params != NULL) {
        TABS(tabs);
        printf("Parameters (%d)\n", params->definitionsCount);
        for (int i = 0; i < params->definitionsCount; i++) {
            print_var_definition(params->definitions[i], tabs + 1);
        }
    }
}

void print_statement (ast_statement* statement, int tabs) {
    if (statement != NULL) {
        if (statement->type == STM_GROUP) {
            ast_statement_group* group = (ast_statement_group*) statement;
            TABS(tabs);
            printf("Statements (%d)\n", group->stmsCount);
            for (int i = 0; i < group->stmsCount; i++) {
                print_statement(group->stms[i], tabs);
            }
        } else if (statement->type == STM_VARDEF) {
            print_var_definition((ast_var_definition*)statement, tabs + 1);
        } else if (statement->type == STM_RETURN) {
            ast_statement_return* ret = (ast_statement_return*) statement;
            TABS(tabs);
            printf("return -->\n");
            TABS(tabs + 1);
            print_expression(ret->exp, tabs + 1);
        }
    }
}

void print_expression (ast_expression* expression, int tabs) {
    if (expression->type == EXP_NUMBER) {
        ast_expression_number* exp_num = (ast_expression_number*) expression;
        printf("%s", exp_num->value);
    } else if (expression->type == EXP_VARIABLE) {
        ast_variable* exp_var = (ast_variable*) expression;
        printf("%s", exp_var->name);
        //TODO: keep the recursion
    } else if (expression->type == EXP_UNOP) {
        ast_expression_unop* exp_unop = (ast_expression_unop*) expression;
        printf("(- ");
        print_expression(exp_unop->child, tabs);
        printf(")");
    } else if (expression->type == EXP_BINOP) {
        print_expression_binop((ast_expression_binop*)expression, tabs);
    } else if (expression->type == EXP_TYPE) {
        ast_expression_type* exp_type = (ast_expression_type*) expression;
        printf(" type\n", exp_type->params->definitionsCount);
        print_paramsdef(exp_type->params, tabs + 1);
        print_statement(exp_type->body, tabs + 1);
    } else if (expression->type == EXP_FUNCTION) {
        ast_expression_function* exp_function = (ast_expression_function*) expression;
        printf(" function\n", exp_function->params->definitionsCount);
        print_paramsdef(exp_function->params, tabs + 1);
        print_statement(exp_function->body, tabs + 1);
    }
}

int main () {
    Lexer* lexer = lexer_create("test/demo.li");

    ast_statement* stm = parse_statement(lexer);
    while (stm != NULL) {
		print_statement(stm, 0);
		stm = parse_statement(lexer);
	}
    printf("\nExit Programm :)\n");

    lexer_destroy(lexer);
    return 0;
}
