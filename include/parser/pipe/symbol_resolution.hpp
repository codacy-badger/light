#pragma once

#include "parser/pipes.hpp"

#include <string.h>
#include <vector>
#include <map>

using namespace std;

void try_replace_ident_by_const (Ast_Ident** ident_ptr2) {
    auto decl = (*ident_ptr2)->declaration;
    if (decl && decl->is_constant()) {
        auto _addr = reinterpret_cast<Ast_Expression**>(ident_ptr2);
        delete *_addr;
        (*_addr) = decl->expression;
    }
}

bool try_resolve (Ast_Ident** ident_ptr2, Ast_Declaration* decl) {
    if (strcmp((*ident_ptr2)->name, decl->name) == 0) {
        if ((*ident_ptr2)->scope->is_ancestor(decl->scope)) {
            (*ident_ptr2)->declaration = decl;
            try_replace_ident_by_const(ident_ptr2);
            return true;
        }
    }
    return false;
}

struct Symbol_Resolution : Pipe {
    map<Ast_Statement*, vector<Ast_Ident**>> unresolved;
	vector<Ast_Ident**> collected_ident_ptrs;

	PIPE_NAME(Symbol_Resolution)

	void on_statement(Ast_Statement** stm) {
	    this->collected_ident_ptrs.clear();

		auto start = os_get_time();
	    Pipe::handle(stm);
		if ((*stm)->stm_type == AST_STATEMENT_DECLARATION) {
			auto decl = static_cast<Ast_Declaration*>((*stm));
			auto it = this->collected_ident_ptrs.begin();
			while (it != this->collected_ident_ptrs.end()) {
	            if (try_resolve(*it, decl)) {
	                it = this->collected_ident_ptrs.erase(it);
	            } else it++;
			}
		}
		this->accumulated_spans += os_clock_stop(start);

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
	        g_compiler->stop();
	        return;
	    }
	}

	void on_resolved (Ast_Statement** stm) {
	    this->to_next(stm);
	    if ((*stm)->stm_type == AST_STATEMENT_DECLARATION) {
	        auto decl = static_cast<Ast_Declaration*>(*stm);
			if (decl->is_constant()) {
	            vector<Ast_Statement*> resolved_stms;

	            for (auto &stm_idents : this->unresolved) {
	                auto ident_ptr2 = stm_idents.second.begin();
	                while (ident_ptr2 != stm_idents.second.end()) {
	                    if (try_resolve(*ident_ptr2, decl)) {
	                        ident_ptr2 = stm_idents.second.erase(ident_ptr2);
	                    } else ident_ptr2++;
	                }

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

	bool is_unresolved (const char* name) {
	    for (auto stm_idents : this->unresolved) {
	        if (stm_idents.first->stm_type == AST_STATEMENT_DECLARATION) {
	            auto decl = static_cast<Ast_Declaration*>(stm_idents.first);
	            if (strcmp(decl->name, name) == 0) {
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
				} else try_replace_ident_by_const(ident_ptr);
			} else this->collected_ident_ptrs.push_back(ident_ptr);
		}
	}
};
