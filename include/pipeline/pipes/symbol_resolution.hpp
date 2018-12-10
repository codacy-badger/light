#pragma once

#include "pipeline/pipe.hpp"

#include <vector>
#include <map>

#include "compiler.hpp"

using namespace std;

void try_resolve_idents (vector<Ast_Ident**>* idents, Ast_Declaration* decl) {
    auto it = idents->begin();
    while (it != idents->end()) {
        auto ident_ptr = (*it);
        auto ident = (*ident_ptr);

        ASSERT(ident->exp_type == AST_EXPRESSION_IDENT);

        if (ident->exp_type == AST_EXPRESSION_IDENT) {
            if (strcmp(ident->name, decl->name) == 0
                    && ident->scope->is_ancestor(decl->scope)) {
                ident->declaration = decl;
                it = idents->erase(it);
            } else it++;
        } else it = idents->erase(it);
    }
}

struct Symbol_Resolution : Pipe {
    map<Ast_Statement*, vector<Ast_Ident**>> unresolved;
	vector<Ast_Ident**> collected_ident_ptrs;

	PIPE_NAME(Symbol_Resolution)

	void on_statement(Ast_Statement** stm) {
	    this->collected_ident_ptrs.clear();

		auto start = os_get_user_time();
	    Pipe::handle(stm);
		if ((*stm)->stm_type == AST_STATEMENT_DECLARATION) {
			auto decl = static_cast<Ast_Declaration*>((*stm));
            try_resolve_idents(&this->collected_ident_ptrs, decl);
		}
		this->total_time += os_time_user_stop(start);

        this->stop_processing = true;
	    if (this->collected_ident_ptrs.size() > 0) {
	        this->unresolved[(*stm)] = this->collected_ident_ptrs;
	    } else this->on_resolved(stm);
	}

	void find_unique_unresolved (vector<Ast_Ident*>* idents) {
	    for (auto deps : this->unresolved) {
	        for (auto ident_ptr : deps.second) {
	            idents->push_back(*ident_ptr);
	        }
	    }

	    for (auto deps : this->unresolved) {
	        if (deps.first->stm_type == AST_STATEMENT_DECLARATION) {
	            auto decl = static_cast<Ast_Declaration*>(deps.first);
	            auto ident_ptr = idents->begin();
	            while (ident_ptr != idents->end()) {
	                if (strcmp(decl->name, (*ident_ptr)->name) == 0
	                    && (*ident_ptr)->scope->is_ancestor(decl->scope)) {
	                    ident_ptr = idents->erase(ident_ptr);
	                } else ident_ptr++;
	            }
	        }
	    }
	}

	void on_finish () {
	    if (this->unresolved.size() > 0) {
	        vector<Ast_Ident*> idents;
	        this->find_unique_unresolved(&idents);
	        for (auto ident : idents) {
	            report_error(&ident->location, "Unresolved symbol: '%s'", ident->name);
	        }
	        Compiler::instance->quit();
	    }
	}

	void on_resolved (Ast_Statement** stm) {
	    this->pending_stms.push_back(*stm);

	    if ((*stm)->stm_type == AST_STATEMENT_DECLARATION) {
	        auto decl = static_cast<Ast_Declaration*>(*stm);
			if (decl->is_constant()) {
	            vector<Ast_Statement*> resolved_stms;

	            for (auto &stm_idents : this->unresolved) {
                    try_resolve_idents(&stm_idents.second, decl);

	                if (stm_idents.second.size() == 0) {
	                    resolved_stms.push_back(stm_idents.first);
	                }
	            }

	            for (auto &resolved_stm : resolved_stms) {
	                this->unresolved.erase(resolved_stm);
	                this->on_resolved(&resolved_stm);
	            }
			}
	    }
	}

	bool is_unresolved (const char* decl_name) {
	    for (auto stm_idents : this->unresolved) {
	        if (stm_idents.first->stm_type == AST_STATEMENT_DECLARATION) {
	            auto decl = static_cast<Ast_Declaration*>(stm_idents.first);
	            if (strcmp(decl->name, decl_name) == 0) {
	                return true;
	            }
	        }
	    }
	    return false;
	}

	void handle (Ast_Binary** binary_ptr) {
		auto binary = (*binary_ptr);

		if (binary->binary_op == AST_BINARY_ATTRIBUTE) {
			// We don't resolve symbols for struct attributes here,
			// since we still don't know the type of the lhs.
			Pipe::handle(&binary->lhs);
		} else Pipe::handle(binary_ptr);
	}

	void handle (Ast_Ident** ident_ptr) {
		auto ident = (*ident_ptr);

		if (!ident->declaration) {
			ident->declaration = ident->scope->find_const_declaration(ident->name);
			if (ident->declaration) {
				if (this->is_unresolved(ident->name)) {
					this->collected_ident_ptrs.push_back(ident_ptr);
				}
			} else this->collected_ident_ptrs.push_back(ident_ptr);
		}
	}

    // We don't want to keep recursing on inner functions,
    // since that would make more than 1 statement depend on the same identifiers
    // Good: [func -> inner_func] [inner_func -> some_ident]
    // Bad: [func -> (inner_func, some_ident)] [inner_func -> some_ident]
    void handle (Ast_Function** func_ptr) { Pipe::handle(func_ptr); }
};
