#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"

#define PUSH_LVAL(new_val) auto lval_tmp = is_left_value; is_left_value = new_val;
#define POP_LVAL is_left_value = lval_tmp;

enum Bytecode_Type : uint8_t {
    BYTECODE_TYPE_UNDEFINED = 0,

    BYTECODE_TYPE_BOOL,

    BYTECODE_TYPE_U8,
    BYTECODE_TYPE_U16,
    BYTECODE_TYPE_U32,
    BYTECODE_TYPE_U64,

    BYTECODE_TYPE_S8,
    BYTECODE_TYPE_S16,
    BYTECODE_TYPE_S32,
    BYTECODE_TYPE_S64,

    BYTECODE_TYPE_F32,
    BYTECODE_TYPE_F64,

    BYTECODE_TYPE_POINTER,
};

struct Generate_Bytecode : Compiler_Pipe<Ast_Statement*>, Ast_Navigator {
    uint64_t next_register = 0;
    uint64_t stack_offset = 0;

    Array<Instruction*>* bytecode = NULL;

    bool is_left_value = false;

    Generate_Bytecode() : Compiler_Pipe("Generate Bytecode") { /* empty */ }

    void handle (Ast_Statement* global_statement) {
        Ast_Navigator::ast_handle(global_statement);
        this->push_out(global_statement);
    }

    void ast_handle (Ast_Scope* scope) {
        printf("PUSH_SCOPE\n");
        Ast_Navigator::ast_handle(scope);
        printf("POP_SCOPE\n");
    }

    void ast_handle (Ast_Declaration* decl) {
        if (!decl->is_constant) {
            For (decl->names) {
                auto value_at = this->get_value_at(decl, i);
                auto type_at = this->get_type_at(decl, i);
                auto var_size = type_at->byte_size;

                decl->reg = this->next_register++;
                printf("ALLOCATE %zd, %zd\n", decl->reg, var_size);

                if (value_at) {
                    Ast_Navigator::ast_handle(value_at);

                    if (this->should_be_in_register(value_at->inferred_type)) {
                        printf("STORE %zd, %zd\n", decl->reg, value_at->reg);
                    } else {
                        printf("COPY_MEM %zd, %zd\n", decl->reg, value_at->reg);
                    }
                }
            }
        } else if (decl->value->exp_type == AST_EXPRESSION_FUNCTION) {
            auto func = static_cast<Ast_Function*>(decl->value);
            this->ensure_bytecode_for_function(func);
        }
    }

    void ast_handle (Ast_Assign* assign) {
        PUSH_LVAL(true);
        Ast_Navigator::ast_handle(assign->variable);
        POP_LVAL;

        Ast_Navigator::ast_handle(assign->value);

        if (this->should_be_in_register(assign->value->inferred_type)) {
            printf("STORE %zd, %zd\n", assign->variable->reg, assign->value->reg);
        } else {
            printf("COPY_MEM %zd, %zd\n", assign->variable->reg, assign->value->reg);
        }
    }

    void ast_handle (Ast_Expression* exp) {
        assert(exp->inferred_type);

        Ast_Navigator::ast_handle(exp);
    }

    void ast_handle (Ast_Ident* ident) {
        assert(ident->declaration);

        if (ident->declaration->is_constant) {
            Ast_Navigator::ast_handle(ident->declaration->value);
            return;
        }
        
        if (ident->declaration->is_global()) {
            auto offset = ident->declaration->global_offset;
            assert(offset > 0);

            printf("GLOBAL_OFFSET %zd, %zd\n", ident->reg, offset);
        }
        
        ident->reg = this->next_register++;
        if (this->should_be_in_register(ident->inferred_type)) {
            printf("LOAD %zd, %zd\n", ident->reg, ident->declaration->reg);
        } else {
            printf("COPY %zd, %zd\n", ident->reg, ident->declaration->reg);
        }
    }

    void ast_handle (Ast_Type*) { /* empty */ }

    void ast_handle (Ast_Function_Call* call) {
        Ast_Navigator::ast_handle(call->func);

        assert(call->func->inferred_type);
        assert(call->func->inferred_type->typedef_type == AST_TYPEDEF_FUNCTION);
        auto func_type = static_cast<Ast_Function_Type*>(call->func->inferred_type);

        auto arg_count = func_type->arg_types.size();
        auto return_count = this->get_return_count(func_type);
        printf("CALL_SETUP %zd, %zd, (convention)\n", arg_count, return_count);

        For (call->arguments->expressions) {
            Ast_Navigator::ast_handle(it);
            printf("CALL_SET_ARGUMENT %zd, %zd\n", i, it->reg);
        }

        printf("CALL %zd\n", call->func->reg);

        call->ret_regs.resize(return_count);
        for (size_t i = 0; i < return_count; i++ ) {
            auto ret_reg = this->next_register++;
            printf("CALL_RETURN %zd, %zd\n", ret_reg, i);
            call->ret_regs[i] = ret_reg;
        }

        if (return_count > 0) {
            call->reg = call->ret_regs[0];
        }
    }

    void ast_handle (Ast_Run* run) {
        this->ensure_bytecode_for_run_directive(run);

        // @TODO run directive NOW
        // @TODO if inferred type is not void...
        // @TODO ...get run result from interpreter

        assert(run->inferred_type);
        if (run->inferred_type->byte_size > 0) {
            run->reg = this->next_register++;
            auto size = run->inferred_type->byte_size;
            printf("SET_CONST %zd, %zd, (run result)\n", run->reg, size);
        }
    }

    void ast_handle (Ast_Function* func) {
        if (func->is_native()) return;

        this->ensure_bytecode_for_function(func);
        
        func->reg = this->next_register++;
        printf("SET_FUNC_PTR %zd, %p\n", func->reg, func);
    }

    void ast_handle (Ast_Cast* cast) {
        Ast_Navigator::ast_handle(cast->value);

        assert(cast->value->inferred_type);
        assert(cast->cast_to->exp_type == AST_EXPRESSION_TYPE);
        auto target_type = this->get_bytecode_type(cast->typed_cast_to);
        auto source_type = this->get_bytecode_type(cast->value->inferred_type);

        cast->reg = this->next_register++;
        printf("CAST %zd, %d, %zd, %d\n",
            cast->reg, target_type, cast->value->reg, source_type);
    }

    void ast_handle (Ast_Binary* binary) {
        assert(binary->inferred_type);
        auto target_type = this->get_bytecode_type(binary->inferred_type);
        
        switch (binary->binary_op) {
            case AST_BINARY_ATTRIBUTE: {
                PUSH_LVAL(true);
                Ast_Navigator::ast_handle(binary->lhs);
                POP_LVAL;

                assert(binary->rhs->exp_type == AST_EXPRESSION_IDENT);
                auto attr = static_cast<Ast_Ident*>(binary->rhs);
                assert(attr->declaration);
                auto attr_offset = attr->declaration->attribute_byte_offset;

                binary->reg = this->next_register++;
                printf("ADD_CONST %zd, %zd, %zd, %d\n",
                    binary->reg, binary->lhs->reg, attr_offset, target_type);
                break;
            }
            case AST_BINARY_SUBSCRIPT: {
                PUSH_LVAL(true);
                Ast_Navigator::ast_handle(binary->lhs);
                POP_LVAL;
                Ast_Navigator::ast_handle(binary->rhs);

                binary->reg = this->next_register++;
                printf("ADD %zd, %zd, %zd, %d\n",
                    binary->reg, binary->lhs->reg, binary->rhs->reg, target_type);
                break;
            }
            default: {
                Ast_Navigator::ast_handle(binary->lhs);
                Ast_Navigator::ast_handle(binary->rhs);

                binary->reg = this->next_register++;
                printf("(some binop) %zd, %zd, %zd, %d\n",
                    binary->reg, binary->lhs->reg, binary->rhs->reg, target_type);
                break;
            }
        }
    }

    void ast_handle (Ast_Unary* unary) {
        switch (unary->unary_op) {
            case AST_UNARY_DEREFERENCE: {
                Ast_Navigator::ast_handle(unary->exp);
                
                unary->reg = this->next_register++;
                printf("LOAD %zd, %zd\n", unary->reg, unary->exp->reg);
                break;
            }
            case AST_UNARY_REFERENCE: {
                PUSH_LVAL(true);
                Ast_Navigator::ast_handle(unary->exp);
                POP_LVAL;
                
                unary->reg = this->next_register++;
                printf("COPY %zd, %zd\n", unary->reg, unary->exp->reg);
                break;
            }
            case AST_UNARY_NEGATE: {
                Ast_Navigator::ast_handle(unary->exp);
                
                unary->reg = this->next_register++;
                printf("NEGATE %zd, %zd\n", unary->reg, unary->exp->reg);
                break;
            }
            case AST_UNARY_NOT: {
                Ast_Navigator::ast_handle(unary->exp);
                
                unary->reg = this->next_register++;
                printf("NOT %zd, %zd\n", unary->reg, unary->exp->reg);
                break;
            }
        }
    }

    void ast_handle (Ast_Literal* literal) {
        literal->reg = this->next_register++;
        auto size = literal->inferred_type->byte_size;
        switch (literal->literal_type) {
            case AST_LITERAL_UNSIGNED_INT: {
                printf("SET_CONST %zd, %zd, %llu\n", literal->reg, size, literal->uint_value);
                break;
            }
            case AST_LITERAL_SIGNED_INT: {
                printf("SET_CONST %zd, %zd, %lld\n", literal->reg, size, literal->int_value);
                break;
            }
            case AST_LITERAL_DECIMAL: {
                printf("SET_CONST %zd, %zd, %lf\n", literal->reg, size, literal->decimal_value);
                break;
            }
            case AST_LITERAL_BOOL: {
                printf("SET_CONST %zd, %zd, %d\n", literal->reg, size, literal->bool_value);
                break;
            }
            case AST_LITERAL_STRING: {
                assert(false);
                break;
            }
        }
    }

    void ensure_bytecode_for_run_directive (Ast_Run* run) {
        if (run->bytecode.size > 0) return;

        this->context->debug(run, "Generate bytecode for run directive...");

        auto tmp1 = this->next_register;
        this->next_register = 0;
        auto tmp2 = this->stack_offset;
        this->stack_offset = 0;

        Ast_Navigator::ast_handle(run);

        this->stack_offset = tmp2;
        this->next_register = tmp1;

        // @DEBUG to prevent functions to run this all the time
        run->bytecode.push(NULL);

        printf("\n...DONE\n\n");
    }

    void ensure_bytecode_for_function (Ast_Function* func) {
        if (func->bytecode.size > 0) return;

        auto unique_func_name = this->build_unique_name(func);
        this->context->debug(func, "Generate bytecode for function '%s' (%s)...",
            func->name, unique_func_name);

        // @DEBUG to prevent functions to run this all the time
        func->bytecode.push(NULL);

        auto tmp1 = this->next_register;
        this->next_register = 0;
        auto tmp2 = this->stack_offset;
        this->stack_offset = 0;

        for (auto stm : func->arg_scope->statements) {
            assert(stm->stm_type == AST_STATEMENT_DECLARATION);
            auto decl = static_cast<Ast_Declaration*>(stm);
            decl->reg = this->next_register++;
        }

        printf("PUSH_FUNC %p\n", func);
        Ast_Navigator::ast_handle(func);

        this->stack_offset = tmp2;
        this->next_register = tmp1;

        printf("\n...DONE\n\n");
    }

    uint8_t get_bytecode_type (Ast_Type* type) {
        if (type == Types::type_bool)   return BYTECODE_TYPE_BOOL;
        if (type == Types::type_u8)     return BYTECODE_TYPE_U8;
        if (type == Types::type_u16)    return BYTECODE_TYPE_U16;
        if (type == Types::type_u32)    return BYTECODE_TYPE_U32;
        if (type == Types::type_u64)    return BYTECODE_TYPE_U64;
        if (type == Types::type_s8)     return BYTECODE_TYPE_S8;
        if (type == Types::type_s16)    return BYTECODE_TYPE_S16;
        if (type == Types::type_s32)    return BYTECODE_TYPE_S32;
        if (type == Types::type_s64)    return BYTECODE_TYPE_S64;
        if (type == Types::type_f32)    return BYTECODE_TYPE_F32;
        if (type == Types::type_f64)    return BYTECODE_TYPE_F64;

        switch (type->typedef_type) {
            case AST_TYPEDEF_FUNCTION:
            case AST_TYPEDEF_POINTER: {
                return BYTECODE_TYPE_POINTER;
            }
            default: break;
        }

        return BYTECODE_TYPE_UNDEFINED;
    }

	char* build_unique_name (Ast_Function* func) {
		auto name_length = strlen(func->name) + 23;
		auto unique_name = new char[name_length];
		sprintf_s(unique_name, name_length, "$%s$%zd", func->name, func->ast_guid);
		return unique_name;
	}

    Ast_Type* get_type_at (Ast_Declaration* decl, size_t index) {
		if (decl->typed_type->typedef_type == AST_TYPEDEF_TUPLE) {
			auto tuple_type = static_cast<Ast_Tuple_Type*>(decl->typed_type);
			return tuple_type->typed_types[index];
		} else if (index == 0) {
			return decl->typed_type;
		} else return NULL;
	}

    Ast_Expression* get_value_at (Ast_Declaration* decl, size_t index) {
        if (!decl->value) return NULL;

		if (decl->value->exp_type == AST_EXPRESSION_COMMA_SEPARATED) {
			auto comma_separated = static_cast<Ast_Comma_Separated*>(decl->value);
			return comma_separated->expressions[index];
		} else if (index == 0) {
			return decl->value;
		} else return NULL;
	}

    size_t get_return_count (Ast_Function_Type* func_type) {
        if (func_type->typed_ret_type->typedef_type == AST_TYPEDEF_TUPLE) {
			auto tuple_type = static_cast<Ast_Tuple_Type*>(func_type->typed_ret_type);
			return tuple_type->types.size;
		} else return 1;
    }

    bool should_be_in_register (Ast_Type* type) {
        return !this->is_left_value && type->is_primitive;
    }
};
