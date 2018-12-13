#pragma once

#include "pipeline/pipe.hpp"

#include "compiler.hpp"

struct Compiler_Directives : Pipe {
	PIPE_NAME(Compiler_Directives)

	Ast_Statement* current_stm = NULL;
    Ast_Scope* current_scope = NULL;

    void handle (Ast_Directive_Include** include_ptr) {
		auto include = (*include_ptr);

		auto location = get_current_stm_location();
		location = this->current_scope->statements.erase(location);

		this->total_time += os_time_user_stop(this->start_time);
		auto scope = Compiler::instance->modules->get_module(include->absolute_path);
		this->start_time = os_get_user_time();

		this->current_scope->statements.insert(location,
			scope->statements.begin(), scope->statements.end());
    }

    void handle (Ast_Directive_Import** import_ptr) {
		auto import = (*import_ptr);

		this->current_scope->statements.erase(get_current_stm_location());

		this->total_time += os_time_user_stop(this->start_time);
		auto scope = Compiler::instance->modules->get_module(import->absolute_path);
		this->start_time = os_get_user_time();

		this->current_scope->module_scopes.push_back(scope);
    }

    void handle (Ast_Directive_If**) { /* TODO: implement */ }

    void handle (Ast_Scope** block_ptr) {
        auto tmp = this->current_scope;
        this->current_scope = (*block_ptr);
        Pipe::handle(block_ptr);
        this->current_scope = tmp;
    }

    void handle (Ast_Statement** stm_ptr) {
        auto tmp = this->current_stm;
        this->current_stm = (*stm_ptr);
        Pipe::handle(stm_ptr);
        this->current_stm = tmp;
    }

    vector<Ast_Statement*>::iterator get_current_stm_location () {
        return find(this->current_scope->statements.begin(),
            this->current_scope->statements.end(), this->current_stm);
    }
};
