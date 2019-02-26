#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"

#define PUSH_LVAL(new_val) auto lval_tmp = is_left_value; is_left_value = new_val;
#define POP_LVAL is_left_value = lval_tmp;

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
        Ast_Navigator::ast_handle(decl);

        if (!decl->is_constant) {
            For (decl->names) {
                auto type_at = this->get_type_at(decl, i);
                auto var_size = type_at->byte_size;

                decl->reg = this->next_register++;
                printf("(dedicated decl register: %zd, %zd)\n", decl->reg, var_size);

                // @TODO ensure the stack offset is aligned as needed

                printf("(TODO: load decl value into reg (or stack?))\n");
            }
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
        
        ident->reg = this->next_register++;
        
        if (ident->declaration->is_global()) {
            auto offset = ident->declaration->global_offset;
            assert(offset > 0);

            printf("GLOBAL_OFFSET %zd, %zd\n", ident->reg, offset);
        }

        if (this->should_be_in_register(ident->inferred_type)) {
            printf("LOAD %zd, %zd\n", ident->reg, ident->declaration->reg);
        }
    }

    void ast_handle (Ast_Type*) { /* empty */ }

    void ast_handle (Ast_Function_Call* call) {
        Ast_Navigator::ast_handle(call->func);

        printf("CALL_SETUP (TODO)\n");

        Ast_Navigator::ast_handle(call->arguments);

        printf("CALL %zd\n", call->func->reg);
    }

    void ast_handle (Ast_Run* run) {
        this->ensure_bytecode_for_run_directive(run);

        // @TODO run directive NOW
        // @TODO if inferred type is not void...
        // @TODO ...get run result from interpreter
        // @TODO generate instructions for the literal value
    }

    void ast_handle (Ast_Function* func) {
        this->ensure_bytecode_for_function(func);
        
        func->reg = this->next_register++;
        printf("SET_FUNC_PTR %zd, %p\n", func->reg, func);
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
                printf("LOAD_CONST (TODO)\n");
                break;
            }
        }
    }

    bool should_be_in_register (Ast_Type* type) {
        return !this->is_left_value && type->is_primitive;
    }

    void ensure_bytecode_for_run_directive (Ast_Run* run) {
        if (run->bytecode.size > 0) return;

        printf("\nGenerate bytecode for run directive...\n\n");

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
        this->context->debug(func, "\nGenerate bytecode for function '%s' (%s)...",
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
        printf("POP_FUNC\n");

        this->stack_offset = tmp2;

        this->next_register = tmp1;

        printf("\n...DONE\n\n");
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
};
