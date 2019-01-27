#pragma once

#include "ast/ast.hpp"

struct Ast_Cloner {

    static Ast_Expression* clone (Ast_Expression* source) {
        Ast_Expression* output = NULL;
        switch (source->exp_type) {
            case AST_EXPRESSION_BINARY: {
                output = clone(static_cast<Ast_Binary*>(source));
                break;
            }
            case AST_EXPRESSION_CAST: {
                output = clone(static_cast<Ast_Cast*>(source));
                break;
            }
            case AST_EXPRESSION_LITERAL: {
                output = clone(static_cast<Ast_Literal*>(source));
                break;
            }
            default: {
                /*Logger::internal(source, "Trying to clone ast node, but it's not "
                    "implemented for this kind (%d) of ast node yet", source->exp_type);*/
                return NULL;
            }
        }
        output->inferred_type = source->inferred_type;
        return output;
    }

    static Ast_Binary* clone (Ast_Binary* source) {
        auto output = new Ast_Binary(source->binary_op);
        output->rhs = source->rhs;
        output->lhs = source->lhs;
        return output;
    }

    static Ast_Unary* clone (Ast_Unary* source) {
        return new Ast_Unary(source->unary_op, source->exp);
    }

    static Ast_Cast* clone (Ast_Cast* source) {
        auto output = new Ast_Cast();
        output->location = source->location;
        output->value = source->value;
    	output->cast_to = source->cast_to;
    	output->is_array_to_slice_cast = source->is_array_to_slice_cast;
        output->is_value_to_any_cast = source->is_value_to_any_cast;
        return output;
    }

    static Ast_Literal* clone (Ast_Literal* source) {
        auto output = new Ast_Literal();
        output->location = source->location;
        output->literal_type = source->literal_type;
        output->uint_value = source->uint_value;
        return output;
    }
};
