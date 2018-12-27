#include "ast/constants.hpp"

#include "compiler.hpp"

Ast_Expression* Constants::value_false = ast_make_literal(false);
Ast_Expression* Constants::value_true = ast_make_literal(true);
