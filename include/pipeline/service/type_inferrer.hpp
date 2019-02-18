#pragma once

#include "ast/nodes.hpp"
#include "utils/ast_navigator.hpp"

#include "ast/types.hpp"

#include "pipeline/service/type_caster.hpp"
#include "pipeline/service/type_table.hpp"

struct Type_Inferrer {
    Build_Context* context = NULL;

    void init (Build_Context* c) { this->context = c; }

    void infer (Ast_Expression* exp) {
        if (exp->inferred_type) return;

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
            case AST_EXPRESSION_CAST: {
                this->infer(static_cast<Ast_Cast*>(exp));
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
            case AST_EXPRESSION_RUN: {
                this->infer(static_cast<Ast_Run*>(exp));
                break;
            }
            default: assert(false);
        }
        assert(exp->inferred_type);
    }

    void infer (Ast_Import* import) {
        import->inferred_type = Types::type_namespace;
    }

    void infer (Ast_Function* func) {
        assert(func->type);

        func->inferred_type = func->func_type;
    }

    void infer (Ast_Run* run) {
        assert(run->expression);
        this->infer(run->expression);

        run->inferred_type = run->expression->inferred_type;
    }

    void infer (Ast_Function_Call* call) {
        this->infer(call->func);
        this->infer(call->arguments);

        assert(call->func->inferred_type);
        assert(call->func->inferred_type->typedef_type == AST_TYPEDEF_FUNCTION);
        auto func_type = static_cast<Ast_Function_Type*>(call->func->inferred_type);

        if (func_type->ret_types.size() > 1) {
            // @TODO handle anonymous types or tuples with several types as returns
            call->inferred_type = static_cast<Ast_Type*>(func_type->ret_types[0]);
        } else call->inferred_type = static_cast<Ast_Type*>(func_type->ret_types[0]);
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
                assert(false);
                break;
            }
            case AST_BINARY_BITWISE_AND:
            case AST_BINARY_BITWISE_OR:
            case AST_BINARY_BITWISE_XOR:
            case AST_BINARY_BITWISE_RIGHT_SHIFT:
            case AST_BINARY_BITWISE_LEFT_SHIFT:
            case AST_BINARY_ADD:
            case AST_BINARY_SUB:
            case AST_BINARY_MUL:
            case AST_BINARY_DIV:
            case AST_BINARY_REM: {
                auto result = this->context->type_caster->try_bidirectional_implicid_cast(&binary->lhs, &binary->rhs);
                if (!result) {
                    this->context->error(binary, "Incompatible types on binary expression: '%s' and '%s'",
                        binary->lhs->inferred_type->name, binary->rhs->inferred_type->name);
                } else binary->inferred_type = binary->lhs->inferred_type;

                break;
            }
        	case AST_BINARY_LOGICAL_AND:
            case AST_BINARY_LOGICAL_OR:
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

    void infer (Ast_Unary* unary) {
        // @Fixme this is a redundant call, since we also call this from type checked
        this->infer(unary->exp);
        assert(unary->exp->inferred_type);
        switch (unary->unary_op) {
            case AST_UNARY_DEREFERENCE: {
                auto type = unary->exp->inferred_type;
                if (type->typedef_type == AST_TYPEDEF_POINTER) {
                    auto ptr_type = static_cast<Ast_Pointer_Type*>(type);

                    assert(ptr_type->base->exp_type == AST_EXPRESSION_TYPE);
                    auto base_type = static_cast<Ast_Type*>(ptr_type->base);
                    unary->inferred_type = base_type;
                }
                break;
            }
        	case AST_UNARY_REFERENCE: {
                auto base_type = unary->exp->inferred_type;
                auto ptr_type = this->context->type_table->get_or_add_pointer_type(base_type);
                unary->inferred_type = ptr_type;
                unary->inferred_type->location = unary->location;
                break;
            }
        	case AST_UNARY_NOT: {
                unary->inferred_type = Types::type_bool;
                break;
            }
        	case AST_UNARY_NEGATE: {
                assert(false);
                break;
            }
        }
    }

    void infer (Ast_Arguments* args) {
		for (auto exp : args->unnamed) {
			if (exp) this->infer(exp);
		}
		for (auto exp : args->named) {
			this->infer(exp.second);
		}
	}

    void infer (Ast_Cast* cast) {
        if (cast->inferred_type) return;

        this->infer(cast->value);
        this->infer(cast->cast_to);

        assert(cast->cast_to);
        assert(cast->cast_to->exp_type == AST_EXPRESSION_TYPE);
        auto target_type = static_cast<Ast_Type*>(cast->cast_to);
        cast->inferred_type = target_type;
    }

    void infer (Ast_Ident* ident) {
        if (ident->inferred_type) return;

        assert(ident->declaration);
        assert(ident->declaration->type);
        assert(ident->declaration->type->exp_type == AST_EXPRESSION_TYPE);
        ident->inferred_type = static_cast<Ast_Type*>(ident->declaration->type);
    }

    void infer (Ast_Literal* lit) {
        if (!lit->inferred_type) {
            switch (lit->literal_type) {
                case AST_LITERAL_BOOL: {
                    lit->inferred_type = Types::type_bool;
                    break;
                }
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
