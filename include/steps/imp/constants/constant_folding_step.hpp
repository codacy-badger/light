#pragma once

#include "steps/simple_pipe.hpp"

#include "utils/ast_ref_navigator.hpp"
#include "ast/factory.hpp"

struct Constant_Folding_Step : Simple_Pipe, Ast_Ref_Navigator {

    Constant_Folding_Step() : Simple_Pipe("Constant Folder") { /* empty */ }

    void handle (void* in) {
        auto stm = static_cast<Ast_Statement*>(in);

        Ast_Ref_Navigator::ast_handle(&stm);
        this->pipe_out(in);
    }

    void ast_handle (Ast_Binary** binary_ptr) {
        auto binary = (*binary_ptr);

        if (this->can_fold(binary->binary_op)
                && binary->lhs->exp_type == AST_EXPRESSION_LITERAL
                && binary->rhs->exp_type == AST_EXPRESSION_LITERAL) {
            auto lit_lhs = static_cast<Ast_Literal*>(binary->lhs);
            auto lit_rhs = static_cast<Ast_Literal*>(binary->rhs);

            // @INFO if the literal types don't match we can't fold it
            if (lit_lhs->literal_type != lit_rhs->literal_type) return;

            switch (lit_lhs->literal_type) {
                case AST_LITERAL_SIGNED_INT: {
                    auto folded_value = this->fold(binary->binary_op, lit_lhs->int_value, lit_rhs->int_value);
                    auto new_lit = Ast_Factory::literal(binary->location, folded_value);
                    (*binary_ptr) = (Ast_Binary*) new_lit;
                    break;
                }
                case AST_LITERAL_UNSIGNED_INT: {
                    auto folded_value = this->fold(binary->binary_op, lit_lhs->int_value, lit_rhs->int_value);
                    auto new_lit = Ast_Factory::literal(binary->location, folded_value);
                    (*binary_ptr) = (Ast_Binary*) new_lit;
                    break;
                }
                case AST_LITERAL_DECIMAL: {
                    auto folded_value = this->fold(binary->binary_op, lit_lhs->int_value, lit_rhs->int_value);
                    auto new_lit = Ast_Factory::literal(binary->location, folded_value);
                    (*binary_ptr) = (Ast_Binary*) new_lit;
                    break;
                }
                case AST_LITERAL_STRING: {
                    // @TODO @Incomplete merging string is not yet supported!
                    break;
                }
            }
        }
    }

    bool can_fold (Ast_Binary_Type type) {
        switch (type) {
        	case AST_BINARY_LOGICAL_AND:
        	case AST_BINARY_LOGICAL_OR:
        	case AST_BINARY_ADD:
        	case AST_BINARY_SUB:
        	case AST_BINARY_MUL:
        	case AST_BINARY_DIV:
        	case AST_BINARY_REM:
        	case AST_BINARY_BITWISE_AND:
        	case AST_BINARY_BITWISE_OR:
        	case AST_BINARY_BITWISE_XOR:
        	case AST_BINARY_BITWISE_RIGHT_SHIFT:
        	case AST_BINARY_BITWISE_LEFT_SHIFT:
        	case AST_BINARY_EQ:
        	case AST_BINARY_NEQ:
        	case AST_BINARY_LT:
        	case AST_BINARY_LTE:
        	case AST_BINARY_GT:
        	case AST_BINARY_GTE:                     return true;
            default:                                 return false;
        }
    }

    template<typename T>
    T fold (Ast_Binary_Type type, T v1, T v2) {
        switch (type) {
        	case AST_BINARY_LOGICAL_AND:           return v1 && v2;
        	case AST_BINARY_LOGICAL_OR:            return v1 || v2;
        	case AST_BINARY_ADD:                   return v1 + v2;
        	case AST_BINARY_SUB:                   return v1 - v2;
        	case AST_BINARY_MUL:                   return v1 * v2;
        	case AST_BINARY_DIV:                   return v1 / v2;
        	case AST_BINARY_REM:                   return v1 % v2;
        	case AST_BINARY_BITWISE_AND:           return v1 & v2;
        	case AST_BINARY_BITWISE_OR:            return v1 | v2;
        	case AST_BINARY_BITWISE_XOR:           return v1 ^ v2;
        	case AST_BINARY_BITWISE_RIGHT_SHIFT:   return v1 >> v2;
        	case AST_BINARY_BITWISE_LEFT_SHIFT:    return v1 << v2;
        	case AST_BINARY_EQ:                    return v1 == v2;
        	case AST_BINARY_NEQ:                   return v1 != v2;
        	case AST_BINARY_LT:                    return v1 < v2;
        	case AST_BINARY_LTE:                   return v1 <= v2;
        	case AST_BINARY_GT:                    return v1 > v2;
        	case AST_BINARY_GTE:                   return v1 >= v2;
            default:                               abort();
        }
    }
};
