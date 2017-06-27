#ifndef LIGHT_LANG_PARSER_STRUCT_H
#define LIGHT_LANG_PARSER_STRUCT_H

typedef enum {
    STM_VARIABLE,
    STM_GROUP,
    STM_VARDEF,
    STM_RETURN,
} StatementType;

typedef enum {
    EXP_NUMBER,
    EXP_VARIABLE,
    EXP_UNOP,
    EXP_BINOP,
    EXP_TYPE,
    EXP_FUNCTION,
} ExpType;

typedef enum {
    EXP_UNOP_NEG,
} ExpUnopType;

typedef enum {
    EXP_BINOP_ASSIGN,
    EXP_BINOP_ADD,
    EXP_BINOP_SUB,
    EXP_BINOP_MUL,
    EXP_BINOP_DIV,
} ExpBinopType;

typedef struct {
    ExpType type;
} ast_expression;

typedef struct {
    ast_expression exp;
    char* value;
} ast_expression_number;

typedef struct {
    ast_expression exp;
    ExpUnopType type;
    ast_expression* child;
} ast_expression_unop;

typedef struct {
    ast_expression exp;
    ExpBinopType type;
    ast_expression* lhs;
    ast_expression* rhs;
} ast_expression_binop;

typedef struct {
    ast_expression** parameters;
    int parametersCount;
} ast_params;

typedef struct {
    char* name;
    ast_params* params;
} ast_type;

typedef struct {
    ast_expression exp;
    char* name;
} ast_variable;



typedef struct {
    StatementType type;
} ast_statement;

typedef struct {
    ast_statement stm;
    ast_expression* exp;
} ast_statement_return;

typedef struct {
    ast_statement stm;
    ast_statement** stms;
    int stmsCount;
} ast_statement_group;

typedef struct {
    ast_statement stm;
    ast_type* type;
    char* name;
    ast_expression* expression;
} ast_var_definition;

typedef struct {
    ast_var_definition** definitions;
    int definitionsCount;
} ast_params_def;

typedef struct {
    ast_expression exp;
    ast_params_def* params;
    ast_statement* body;
} ast_expression_type;

typedef struct {
    ast_expression exp;
    ast_params_def* params;
    ast_statement* body;
} ast_expression_function;

#endif
