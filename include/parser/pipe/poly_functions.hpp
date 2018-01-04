#pragma once

#include "parser/pipes.hpp"

#include <map>

using namespace std;

struct Poly_Functions : Pipe {
    map<Ast_Function*, Ast_Expression**> poly_refs;
    bool stm_continue = true;

    void on_statement (Ast_Statement** stm_ptr) {
        this->stm_continue = true;
    	Pipe::handle(stm_ptr);
        if (this->stm_continue) {
            Pipe::to_next(stm_ptr);
        }
    }

	void handle (Ast_Function** func_ptr) {
		auto func = (*func_ptr);

        if (func->is_poly()) {
            this->poly_refs[func];
            printf("Poly function found -> %s\n", func->name);
            this->stm_continue = false;
        }

        Pipe::handle(func_ptr);
	}

	void handle (Ast_Function_Call** call_ptr) {
		auto call = (*call_ptr);

        if (call->fn->exp_type == AST_EXPRESSION_IDENT) {
            auto ident = static_cast<Ast_Ident*>(call->fn);
            for (auto poly_funcs : this->poly_refs) {
                if (strcmp(ident->name, poly_funcs.first->name) == 0) {
                    printf("FOUND! '%s'\n", ident->name);
                    break;
                }
            }
        }

        Pipe::handle(call_ptr);
	}
};
