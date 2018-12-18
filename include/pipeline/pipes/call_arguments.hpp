#pragma once

#include "pipeline/pipe.hpp"

#include "ast/ast.hpp"

struct Call_Arguments : Pipe {
    Call_Arguments () { this->pipe_name = "Call_Arguments"; }

    void handle (Ast_Function_Call** call_ptr) {
		Pipe::handle(call_ptr);

		auto call = (*call_ptr);
		auto func_type = static_cast<Ast_Function_Type*>(call->func->inferred_type);
        auto arg_without_defaults = func_type->count_arguments_without_defaults();

        if (call->arguments->unnamed.size() > func_type->arg_decls.size()) {
            ERROR_STOP(call, "Too many arguments, function has %d, but call has %d",
                func_type->arg_decls.size(), call->arguments->unnamed.size());
        } else if (call->arguments->unnamed.size() < arg_without_defaults) {
            ERROR_STOP(call, "Too few arguments, function requires %d, but call has %d",
                arg_without_defaults, call->arguments->unnamed.size());
        } else {
            auto arg_count = call->arguments->unnamed.size();
            for (auto i = arg_count; i < func_type->arg_decls.size(); i++) {
                auto decl = func_type->arg_decls[i];

                auto named_value = call->arguments->get_named_value(decl->name);
                if (named_value) {
                    call->arguments->unnamed.push_back(named_value);
                } else call->arguments->unnamed.push_back(decl->expression);
            }
        }
	}

    void handle (Ast_Function_Type** func_type_ptr) {
        auto func_type = (*func_type_ptr);

        bool default_args_found = false;
        for (auto decl : func_type->arg_decls) {
            if (decl->expression) {
                default_args_found = true;
            } else if (default_args_found) {
                ERROR_STOP(func_type, "All default arguments must be "
                    "at the end of the functions arguments, "
                    "but argument '%s' has no default value", decl->name);
            }
        }
    }
};
