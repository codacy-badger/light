#pragma once

#include "phase/pipeline/pipe.hpp"

#include "ast/ast.hpp"

struct Call_Arguments : Pipe {
    Call_Arguments () { this->pipe_name = "Call_Arguments"; }

    void handle (Ast_Function_Call** call_ptr) {
		Pipe::handle(call_ptr);

		auto call = (*call_ptr);
        auto unnamed = &call->arguments->unnamed;
		auto func_type = static_cast<Ast_Function_Type*>(call->func->inferred_type);
        //auto arg_without_defaults = func_type->count_arguments_without_defaults();

        if (unnamed->size() > func_type->arg_decls.size()) {
            Logger::error(call, "Too many arguments, function has %d, but call has %d",
                func_type->arg_decls.size(), unnamed->size());
        } else {
            for (auto i = 0; i < func_type->arg_decls.size(); i++) {
                auto decl = func_type->arg_decls[i];
                auto unnamed_value = call->arguments->get_unnamed_value(i);
                auto named_value = call->arguments->get_named_value(decl->name);

                if (unnamed_value == NULL && named_value == NULL) {
                    if (decl->expression) {
                        if (unnamed->size() <= (i + 1)) unnamed->resize(i + 1);
                        (*unnamed)[i] = decl->expression;
                    } else Logger::error(call, "No value provided for argument '%s'", decl->name);
                } else if (unnamed_value != NULL && named_value != NULL) {
                    Logger::error(call, "The parameter '%s' has both named & unnamed values", decl->name);
                } else if (named_value != NULL) {
                    if (unnamed->size() <= (i + 1)) unnamed->resize(i + 1);
                    (*unnamed)[i] = named_value;
                }
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
                Logger::error(func_type, "All default arguments must be "
                    "at the end of the functions arguments, "
                    "but argument '%s' has no default value", decl->name);
            }
        }
    }
};
