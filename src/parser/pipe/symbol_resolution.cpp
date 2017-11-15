#pragma once

#include "parser/pipe/symbol_resolution.hpp"

#include "compiler.hpp"

void Symbol_Resolution::on_statement(Ast_Statement* stm) {
    vector<Ast_Ident**> unresolved_idents;
    check_symbols(stm, &unresolved_idents);
    if (unresolved_idents.size() > 0) {
        auto stm_deps = new Ast_Statement_Dependency();
        stm_deps->unresolved_symbols = unresolved_idents;
        stm_deps->stm = stm;
        for (auto ident : unresolved_idents) {
            this->unresolved_symbols[(*ident)->name].push_back(stm_deps);
        }
    } else this->on_resolved(stm);
}

void Symbol_Resolution::on_finish () {
    if (this->unresolved_symbols.size() > 0) {
		for (auto unresolved : this->unresolved_symbols) {
			for (auto dep : unresolved.second) {
				for (auto ident : dep->unresolved_symbols) {
			        Light_Compiler::inst->error((*ident), "Unresolved symbol: '%s'", (*ident)->name);
				}
				Light_Compiler::inst->stop();
			}
        }
    } else this->try_finish();
}

void Symbol_Resolution::on_resolved (Ast_Statement* stm) {
    this->to_next(stm);
    if (stm->stm_type == AST_STATEMENT_DECLARATION) {
        auto decl = static_cast<Ast_Declaration*>(stm);

        auto it = this->unresolved_symbols.find(decl->name);
        if (it != this->unresolved_symbols.end()) {
            auto deps = it->second;
            this->unresolved_symbols.erase(it);
            for (auto stm_deps : deps) {
                auto it2 = stm_deps->unresolved_symbols.begin();
                Ast_Ident** ident = NULL;
                while (it2 != stm_deps->unresolved_symbols.end()) {
					ident = (*it2);
                    if (strcmp((*ident)->name, decl->name) == 0) {
                        it2 = stm_deps->unresolved_symbols.erase(it2);

                        // INFO: if the declaration we've resolved to is constant
                        // we replace the current Ident with the decl expression.
                        // This should remove the need to handle constants
                        // bytecode / executable (appart from string).
                        if (decl->decl_flags & DECL_FLAG_CONSTANT) {
                            auto exp_ptr = reinterpret_cast<Ast_Expression**>(ident);
                            delete (*exp_ptr);
                            (*exp_ptr) = decl->expression;
                        } else {
                            (*ident)->declaration = decl;
                        }
					} else {
						it2++;
					}
                }
                if (stm_deps->unresolved_symbols.size() == 0) {
                    auto stm = stm_deps->stm;
                    delete stm_deps;
                    this->on_resolved(stm);
                }
            }
        }
    }
}

bool Symbol_Resolution::is_unresolved (const char* name) {
    for (auto entry : this->unresolved_symbols) {
        for (auto stm_deps : entry.second) {
            if (stm_deps->stm->stm_type == AST_STATEMENT_DECLARATION) {
                auto decl = static_cast<Ast_Declaration*>(stm_deps->stm);
                if (strcmp(decl->name, name) == 0) return true;
            }
        }
    }
    return false;
}

void Symbol_Resolution::check_symbols (Ast_Statement* stm, vector<Ast_Ident**>* sym) {
    switch (stm->stm_type) {
        case AST_STATEMENT_DECLARATION:
            check_symbols(static_cast<Ast_Declaration*>(stm), sym);
			break;
        case AST_STATEMENT_BLOCK:
            check_symbols(static_cast<Ast_Block*>(stm), sym);
			break;
        case AST_STATEMENT_RETURN:
            check_symbols(static_cast<Ast_Return*>(stm), sym);
			break;
        default: break;
    }
}

void Symbol_Resolution::check_symbols (Ast_Declaration* decl, vector<Ast_Ident**>* sym) {
    if (decl->type)         check_symbols(&decl->type, sym);
    if (decl->expression)   check_symbols(&decl->expression, sym);
}

void Symbol_Resolution::check_symbols (Ast_Block* block, vector<Ast_Ident**>* sym) {
    for (auto stm : block->list) check_symbols(stm, sym);
}

void Symbol_Resolution::check_symbols (Ast_Return* ret, vector<Ast_Ident**>* sym) {
    if (ret->exp) check_symbols(&ret->exp, sym);
}

void Symbol_Resolution::check_symbols (Ast_Expression** exp, vector<Ast_Ident**>* sym) {
    switch ((*exp)->exp_type) {
        case AST_EXPRESSION_FUNCTION:
            check_symbols(reinterpret_cast<Ast_Function**>(exp), sym);
			break;
		case AST_EXPRESSION_TYPE_DEFINITION:
			check_symbols(reinterpret_cast<Ast_Type_Definition**>(exp), sym);
			break;
		case AST_EXPRESSION_CALL:
			check_symbols(reinterpret_cast<Ast_Function_Call**>(exp), sym);
			break;
        case AST_EXPRESSION_BINARY:
            check_symbols(reinterpret_cast<Ast_Binary**>(exp), sym);
			break;
        case AST_EXPRESSION_UNARY:
            check_symbols(reinterpret_cast<Ast_Unary**>(exp), sym);
			break;
        case AST_EXPRESSION_IDENT: {
            auto ident_ptr = reinterpret_cast<Ast_Ident**>(exp);
            if (!(*ident_ptr)->declaration || this->is_unresolved((*ident_ptr)->name)) {
                sym->push_back(ident_ptr);
            } else {
				if ((*ident_ptr)->declaration) {
					auto decl = (*ident_ptr)->declaration;
					if (decl->decl_flags & DECL_FLAG_CONSTANT) {
						auto exp_ptr = reinterpret_cast<Ast_Expression**>(ident_ptr);
						delete (*exp_ptr);
						(*exp_ptr) = decl->expression;
					}
				}
            }
			break;
        }
        case AST_EXPRESSION_LITERAL: break;
        default: break;
    }
}

void Symbol_Resolution::check_symbols (Ast_Function** fn, vector<Ast_Ident**>* sym) {
    check_symbols(&(*fn)->type, sym);
    check_symbols((*fn)->scope, sym);
}

void Symbol_Resolution::check_symbols (Ast_Type_Definition** tydef, vector<Ast_Ident**>* sym) {
	switch ((*tydef)->typedef_type) {
        case AST_TYPEDEF_STRUCT: {
			check_symbols(reinterpret_cast<Ast_Struct_Type**>(tydef), sym);
			break;
        }
        case AST_TYPEDEF_POINTER: {
            check_symbols(reinterpret_cast<Ast_Pointer_Type**>(tydef), sym);
			break;
		}
        case AST_TYPEDEF_FUNCTION: {
            check_symbols(reinterpret_cast<Ast_Function_Type**>(tydef), sym);
			break;
		}
        default: break;
    }
}

void Symbol_Resolution::check_symbols (Ast_Struct_Type** _struct, vector<Ast_Ident**>* sym) {
    for (auto decl : (*_struct)->attributes) {
		check_symbols(decl, sym);
	}
}

void Symbol_Resolution::check_symbols (Ast_Pointer_Type** ptr_type, vector<Ast_Ident**>* sym) {
    check_symbols(&(*ptr_type)->base, sym);
}

void Symbol_Resolution::check_symbols (Ast_Function_Type** fn_type, vector<Ast_Ident**>* sym) {
    check_symbols(&(*fn_type)->return_type, sym);
	for (int i = 0; i < (*fn_type)->parameter_types.size(); i++) {
		check_symbols(&(*fn_type)->parameter_types[i], sym);
	}
}

void Symbol_Resolution::check_symbols (Ast_Function_Call** call, vector<Ast_Ident**>* sym) {
    check_symbols(&(*call)->fn, sym);
	for (int i = 0; i < (*call)->parameters.size(); i++) {
		check_symbols(&(*call)->parameters[i], sym);
	}
}

void Symbol_Resolution::check_symbols (Ast_Binary** binary, vector<Ast_Ident**>* sym) {
    check_symbols(&(*binary)->rhs, sym);
    check_symbols(&(*binary)->lhs, sym);
}

void Symbol_Resolution::check_symbols (Ast_Unary** unary, vector<Ast_Ident**>* sym) {
    check_symbols(&(*unary)->exp, sym);
}
