#pragma once

#include "ast/nodes.hpp"
#include "utils/ast_navigator.hpp"

#include "ast/types.hpp"

struct Type_Inferrer {

    void infer (Ast_Declaration* decl) {
        if (!decl->type) {
            assert(decl->expression);
            this->infer(decl->expression);
            decl->type = decl->expression->inferred_type;
        }
    }

    void infer (Ast_Expression* exp) {
        switch (exp->exp_type) {
            case AST_EXPRESSION_IMPORT: {
                this->infer(static_cast<Ast_Import*>(exp));
                break;
            }
            case AST_EXPRESSION_FUNCTION: {
                this->infer(static_cast<Ast_Function*>(exp));
                break;
            }
            case AST_EXPRESSION_TYPE: {
                this->infer(static_cast<Ast_Type*>(exp));
                break;
            }
            case AST_EXPRESSION_CALL: {
                this->infer(static_cast<Ast_Function_Call*>(exp));
                break;
            }
            case AST_EXPRESSION_BINARY: {
                this->infer(static_cast<Ast_Binary*>(exp));
                break;
            }
            case AST_EXPRESSION_UNARY: {
                this->infer(static_cast<Ast_Unary*>(exp));
                break;
            }
            case AST_EXPRESSION_IDENT: {
                this->infer(static_cast<Ast_Ident*>(exp));
                break;
            }
            case AST_EXPRESSION_LITERAL: {
                this->infer(static_cast<Ast_Literal*>(exp));
                break;
            }
            default: assert(false);
        }
        //assert(exp->inferred_type);
    }

    void infer (Ast_Import* import) {
        import->inferred_type = Types::type_namespace;
    }

    void infer (Ast_Function* func) {
        assert(func->type);

        func->inferred_type = func->func_type;
    }

    void infer (Ast_Function_Call* call) {
        this->infer(call->func);
        this->infer(call->arguments);

        assert(call->func->inferred_type);
        assert(call->func->inferred_type->typedef_type == AST_TYPEDEF_FUNCTION);
        auto func_type = static_cast<Ast_Function_Type*>(call->func->inferred_type);

        //assert(func_type->ret_type->exp_type == AST_EXPRESSION_TYPE);
        call->inferred_type = static_cast<Ast_Type*>(func_type->ret_type);
    }

    void infer (Ast_Type* type) {
        type->inferred_type = Types::type_type;
    }

    void infer (Ast_Binary* binary) {
        switch (binary->binary_op) {
        	case AST_BINARY_ATTRIBUTE: {
                binary->inferred_type = binary->rhs->inferred_type;
                break;
            }
        	case AST_BINARY_SUBSCRIPT: {
                abort();
                break;
            }
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
            case AST_BINARY_BITWISE_LEFT_SHIFT: {
                //assert(binary->lhs->inferred_type == binary->rhs->inferred_type);
                break;
            }
        	case AST_BINARY_EQ:
            case AST_BINARY_NEQ:
            case AST_BINARY_LT:
            case AST_BINARY_LTE:
            case AST_BINARY_GT:
            case AST_BINARY_GTE: {
                binary->inferred_type = Types::type_bool;
                break;
            }
        }
    }

    void infer (Ast_Unary*) { /* TODO */ }

    void infer (Ast_Arguments* args) {
		for (auto exp : args->unnamed) {
			this->infer(exp);
		}
		for (auto exp : args->named) {
			this->infer(exp.second);
		}
	}

    void infer (Ast_Ident* ident) {
        if (ident->inferred_type) return;

        if (ident->declaration) {
            this->infer(ident->declaration);
            ident->inferred_type = static_cast<Ast_Type*>(ident->declaration->type);
        }
    }

    void infer (Ast_Literal* lit) {
        if (!lit->inferred_type) {
            switch (lit->literal_type) {
                case AST_LITERAL_UNSIGNED_INT: {
                    lit->inferred_type = Type_Inferrer::get_smallest_type(lit->uint_value);
                    break;
                }
                case AST_LITERAL_SIGNED_INT: {
                    lit->inferred_type = Type_Inferrer::get_smallest_type(lit->int_value);
                    break;
                }
                case AST_LITERAL_DECIMAL: {
                    lit->inferred_type = Types::type_f64;
                    break;
                }
                case AST_LITERAL_STRING: {
                    lit->inferred_type = Types::type_string;
                    break;
                }
            }
        }
    }

    static Ast_Type* get_container_signed (Ast_Type* unsigned_type) {
        if (unsigned_type == Types::type_u8) {
            return Types::type_s16;
        } else if (unsigned_type == Types::type_u16) {
            return Types::type_s32;
        } else if (unsigned_type == Types::type_u32) {
            return Types::type_s64;
        } else if (unsigned_type == Types::type_u64) {
            return Types::type_s64;
        } else return unsigned_type;
    }

    static Ast_Struct_Type* get_smallest_type (uint64_t value) {
        if (value <= UINT32_MAX) {
            if (value <= UINT16_MAX) {
                if (value <= UINT8_MAX) {
                    return Types::type_u8;
                } else return Types::type_u16;
            } else return Types::type_u32;
        } else return Types::type_u64;
    }

    static Ast_Struct_Type* get_smallest_type (int64_t value) {
        if (value <= INT32_MAX && value >= INT32_MIN) {
            if (value <= INT16_MAX && value >= INT16_MIN) {
                if (value <= INT8_MAX && value >= INT8_MIN) {
                    return Types::type_s8;
                } else return Types::type_s16;
            } else return Types::type_s32;
        } else return Types::type_s64;
    }
};
