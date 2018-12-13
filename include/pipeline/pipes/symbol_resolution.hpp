#pragma once

#include "pipeline/pipe.hpp"

#include <vector>

#include "compiler.hpp"

using namespace std;

struct Symbol_Resolution : Pipe {
    vector<Ast_Scope*> unresolved_scopes;
    Ast_Scope* current_scope = NULL;

	PIPE_NAME(Symbol_Resolution)

    void process (Ast_Scope* scope) {
        Pipe::process(scope);

        if (!scope->unresolved_idents.empty()) {
            this->stop_processing = true;
            this->unresolved_scopes.push_back(scope);
        }
    }

    void handle (Ast_Declaration** decl_ptr) {
        Pipe::handle(decl_ptr);

        auto decl = (*decl_ptr);

        auto scope_ptr = this->unresolved_scopes.begin();
        while (scope_ptr != this->unresolved_scopes.end()) {
            auto scope = (*scope_ptr);

            if (scope->is_ancestor(decl->scope)) {
                auto ident_ptr = scope->unresolved_idents.begin();
                while (ident_ptr != scope->unresolved_idents.end()) {
                    auto ident = (*ident_ptr);

                    if (strcmp(ident->name, decl->name) == 0) {
                        ident_ptr = scope->unresolved_idents.erase(ident_ptr);
                        ident->declaration = decl;
                    }
                }

                if (scope->unresolved_idents.empty()) {
                    scope_ptr = this->unresolved_scopes.erase(scope_ptr);
                    this->pipeline->process(scope, this->pipe_index);
                }
            }
        }
    }

	void handle (Ast_Ident** ident_ptr) {
		auto ident = (*ident_ptr);

		if (!ident->declaration) {
			ident->declaration = ident->scope->find_const_declaration(ident->name);
			if (!ident->declaration) {
				this->current_scope->unresolved_idents.push_back(ident);
			}
		}
	}

	void handle (Ast_Binary** binary_ptr) {
		auto binary = (*binary_ptr);

		if (binary->binary_op == AST_BINARY_ATTRIBUTE) {
			// We don't resolve symbols for struct attributes here,
			// since we still don't know the type of the lhs.
			Pipe::handle(&binary->lhs);
		} else Pipe::handle(binary_ptr);
	}

    void handle (Ast_Scope** scope_ptr) {
        auto tmp = this->current_scope;
        this->current_scope = (*scope_ptr);
        Pipe::handle(scope_ptr);
        this->current_scope = tmp;
    }

    // We don't want to keep recursing on inner functions,
    // since that would make more than 1 statement depend on the same identifiers
    // Good: [func -> inner_func] [inner_func -> some_ident]
    // Bad: [func -> (inner_func, some_ident)] [inner_func -> some_ident]
    void handle (Ast_Function** func_ptr) { Pipe::handle(func_ptr); }

	void on_finish () {
	    if (this->unresolved_scopes.size() > 0) {
	        vector<Ast_Ident*> idents;
	        this->find_unique_unresolved(&idents);
	        for (auto ident : idents) {
	            ERROR(ident, "Unresolved symbol: '%s'", ident->name);
	        }
	        Compiler::instance->quit();
	    }
	}

	void find_unique_unresolved (vector<Ast_Ident*>* idents) {
	    for (auto scope : this->unresolved_scopes) {
	        for (auto ident : scope->unresolved_idents) {
	            idents->push_back(ident);
	        }
	    }

	    for (auto scope : this->unresolved_scopes) {
            auto ident_ptr = idents->begin();
            while (ident_ptr != idents->end()) {
                if ((*ident_ptr)->scope->is_ancestor(scope)) {
                    ident_ptr = idents->erase(ident_ptr);
                } else ident_ptr++;
            }
	    }
	}
};
