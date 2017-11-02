#pragma once

#include "parser/pipe/symbol_resolution.hpp"

#include "compiler.hpp"

void Symbol_Resolution::on_statement(Ast_Statement* stm) {
    set<const char*, cmp_str> unresolved_symbols;
    check_symbols(stm, &unresolved_symbols);
    if (unresolved_symbols.size() > 0) {
        auto stm_deps = new Ast_Statement_Dependency();
        stm_deps->unresolved_symbols = unresolved_symbols;
        stm_deps->stm = stm;
        for (auto symbol : unresolved_symbols) {
            this->unresolved_symbols[symbol].insert(stm_deps);
        }
    } else {
        this->to_next(stm);
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
    		auto it3 = this->unresolved_symbols.find(decl->name);
    		if (it3 != this->unresolved_symbols.end()) {
                auto deps = it3->second;
				this->unresolved_symbols.erase(decl->name);
    			for (auto stm_deps : deps) {
                    stm_deps->unresolved_symbols.erase(decl->name);
                    if (stm_deps->unresolved_symbols.size() == 0) {
                        this->on_statement(stm_deps->stm);
                        delete stm_deps;
                    }
                }
    		}
        }
    }
}

void Symbol_Resolution::on_finish () {
    if (this->unresolved_symbols.size() > 0) {
        Light_Compiler::instance->error(NULL, "There are unresolved symbols!");
    } else this->try_finish();
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

void Symbol_Resolution::check_symbols (Ast_Statement* stm, set<const char*, cmp_str>* sym) {
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

void Symbol_Resolution::check_symbols (Ast_Declaration* decl, set<const char*, cmp_str>* sym) {
    if (decl->type)         check_symbols(decl->type, sym);
    if (decl->expression)   check_symbols(decl->expression, sym);

	if (sym->size() == 0) {
		auto it2 = this->unresolved_references.find(decl->name);
		if (it2 != this->unresolved_references.end()) {
			for (auto ref : this->unresolved_references[decl->name]) {
				*ref = decl;
			}
			this->unresolved_references.erase(decl->name);
		}
	}
}

void Symbol_Resolution::check_symbols (Ast_Type_Definition* tydef, set<const char*, cmp_str>* sym) {
	switch (tydef->typedef_type) {
        case AST_TYPEDEF_STRUCT: {
            auto struct_type = static_cast<Ast_Struct_Type*>(tydef);
			break;
        }
        case AST_TYPEDEF_POINTER: {
            check_symbols(static_cast<Ast_Pointer_Type*>(tydef), sym);
			break;
		}
        case AST_TYPEDEF_FUNCTION: {
            check_symbols(static_cast<Ast_Function_Type*>(tydef), sym);
			break;
		}
        default: break;
    }
}

void Symbol_Resolution::check_symbols (Ast_Block* block, set<const char*, cmp_str>* sym) {
    for (auto stm : block->list) check_symbols(stm, sym);
}

void Symbol_Resolution::check_symbols (Ast_Return* ret, set<const char*, cmp_str>* sym) {
    if (ret->exp) check_symbols(ret->exp, sym);
}

void Symbol_Resolution::check_symbols (Ast_Expression* exp, set<const char*, cmp_str>* sym) {
    switch (exp->exp_type) {
        case AST_EXPRESSION_FUNCTION:
            check_symbols(static_cast<Ast_Function*>(exp), sym);
			break;
        case AST_EXPRESSION_BINARY:
            check_symbols(static_cast<Ast_Binary*>(exp), sym);
			break;
        case AST_EXPRESSION_UNARY:
            check_symbols(static_cast<Ast_Unary*>(exp), sym);
			break;
        case AST_EXPRESSION_IDENT: {
            check_symbols(static_cast<Ast_Ident*>(exp), sym);
			break;
        }
        case AST_EXPRESSION_LITERAL: break;
        default: break;
    }
}

void Symbol_Resolution::check_symbols (Ast_Ident* ident, set<const char*, cmp_str>* sym) {
    if (!ident->declaration) {
        this->unresolved_references[ident->name].insert(&ident->declaration);
        sym->insert(ident->name);
    } else if (this->is_unresolved(ident->name)) {
        sym->insert(ident->name);
    }
}

void Symbol_Resolution::check_symbols (Ast_Binary* binary, set<const char*, cmp_str>* sym) {
    check_symbols(binary->rhs, sym);
    check_symbols(binary->lhs, sym);
}

void Symbol_Resolution::check_symbols (Ast_Unary* unary, set<const char*, cmp_str>* sym) {
    check_symbols(unary->exp, sym);
}

void Symbol_Resolution::check_symbols (Ast_Function* fn, set<const char*, cmp_str>* sym) {
    check_symbols(fn->type, sym);
    check_symbols(fn->scope, sym);
}

void Symbol_Resolution::check_symbols (Ast_Pointer_Type* ptr_type, set<const char*, cmp_str>* sym) {
    check_symbols(ptr_type->base, sym);
}

void Symbol_Resolution::check_symbols (Ast_Function_Type* fn_type, set<const char*, cmp_str>* sym) {
    check_symbols(fn_type->return_type, sym);
    for (auto param_type : fn_type->parameter_types)
        check_symbols(param_type, sym);
}
