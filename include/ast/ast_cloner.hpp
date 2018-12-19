#pragma once

#include "ast/ast.hpp"

struct Ast_Cloner {

    static Ast_Literal* clone (Ast_Literal* source) {
        auto output = new Ast_Literal();
        output->literal_type = source->literal_type;
        output->uint_value = source->uint_value;
        return output;
    }

    static Ast_Expression* clone (Ast_Expression* source) {
        Ast_Expression* output = NULL;
        switch (source->exp_type) {
            case AST_EXPRESSION_LITERAL: {
                output = clone(static_cast<Ast_Literal*>(source));
                break;
            }
            default: {
                INTERNAL(source, "Trying to clone ast node, but it's not "
                    "implemented for this kind of ast node yet");
                return NULL;
            }
        }
        output->inferred_type = source->inferred_type;
        return output;
    }
};
