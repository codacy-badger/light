#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"

struct Generate_Bytecode : Compiler_Pipe<Ast_Statement*>, Ast_Navigator {
    uint64_t next_register = 0;

    Generate_Bytecode() : Compiler_Pipe("Generate Bytecode") { /* empty */ }

    void handle (Ast_Statement* global_statement) {
        Ast_Navigator::ast_handle(global_statement);
        this->push_out(global_statement);
    }

    void ast_handle (Ast_Expression* exp) {
        assert(exp->inferred_type);
        Ast_Navigator::ast_handle(exp);
    }

    void ast_handle (Ast_Type*) { /* empty */ }

    void ast_handle (Ast_Function_Call* call) {
        Ast_Navigator::ast_handle(call->func);

        printf("CALL_SETUP (TODO)\n");

        Ast_Navigator::ast_handle(call->arguments);

        printf("CALL %zd\n", call->func->reg);
    }

    void ast_handle (Ast_Function* func) {
        this->ensure_bytecode_for_function(func);
        
        func->reg = this->next_register++;
        auto size = func->inferred_type->byte_size;
        printf("SET_FUNC_PTR %zd, %zd, %p\n", func->reg, size, func);
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

    void ensure_bytecode_for_function (Ast_Function* func) {
        if (func->bytecode.size > 0) return;

        auto unique_func_name = build_unique_name(func);
        printf("\nGenerate bytecode for function '%s' (%s)...\n\n",
            func->name, unique_func_name);

        auto tmp = this->next_register;
        this->next_register = 0;
        Ast_Navigator::ast_handle(func);
        this->next_register = tmp;

        // @DEBUG to prevent functions to run this all the time
        func->bytecode.push(NULL);

        printf("\n...DONE\n\n");
    }

	char* build_unique_name (Ast_Function* func) {
		auto name_length = strlen(func->name) + 23;
		auto unique_name = new char[name_length];
		sprintf_s(unique_name, name_length, "$%s$%zd", func->name, func->ast_guid);
		return unique_name;
	}
};
