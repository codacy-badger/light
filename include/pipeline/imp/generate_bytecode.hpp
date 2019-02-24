#pragma once

#include "pipeline/compiler_pipe.hpp"
#include "utils/ast_navigator.hpp"

struct Generate_Bytecode : Compiler_Pipe<Ast_Statement*>, Ast_Navigator {


    Generate_Bytecode() : Compiler_Pipe("Generate Bytecode") { /* empty */ }

    void handle (Ast_Statement* global_statement) {
        //Ast_Navigator::ast_handle(global_statement);
        this->push_out(global_statement);
    }

    void generate_bytecode_for_function (Ast_Function* func) {
        assert(func->bytecode.size == 0);

        auto unique_func_name = build_unique_name(func);
        printf("\nGenerate bytecode for function '%s' (%s)...\n\n",
            func->name, unique_func_name);

        printf("\n...DONE\n\n");
    }

    void ast_handle (Ast_Expression* exp) {
        //assert(exp->inferred_type);
        Ast_Navigator::ast_handle(exp);
    }

    void ast_handle (Ast_Function* func) {
        Ast_Navigator::ast_handle(func);

        if (!(func->func_flags & FUNCTION_FLAG_BYTECODE_GENERATED)) {
            this->generate_bytecode_for_function(func);
            func->func_flags |= FUNCTION_FLAG_BYTECODE_GENERATED;
        }

        // @TODO generate bytecode to get that function (LOAD_CONST?)
    }

    void ast_handle (Ast_Literal* literal) {
        auto size = literal->inferred_type->byte_size;
        switch (literal->literal_type) {
            case AST_LITERAL_UNSIGNED_INT: {
                printf("SET_CONST %zd, %llu\n", size, literal->uint_value);
                break;
            }
            case AST_LITERAL_SIGNED_INT: {
                printf("SET_CONST %zd, %lld\n", size, literal->int_value);
                break;
            }
            case AST_LITERAL_DECIMAL: {
                printf("SET_CONST %zd, %lf\n", size, literal->decimal_value);
                break;
            }
            case AST_LITERAL_BOOL: {
                printf("SET_CONST %zd, %d\n", size, literal->bool_value);
                break;
            }
            case AST_LITERAL_STRING: {
                printf("LOAD_CONST %zd, (TODO)\n", size);
                break;
            }
        }
        printf("SET_CONST");
    }

	char* build_unique_name (Ast_Function* func) {
		auto name_length = strlen(func->name) + 23;
		auto unique_name = new char[name_length];
		sprintf_s(unique_name, name_length, "$%s$%zd", func->name, func->ast_guid);
		return unique_name;
	}
};
