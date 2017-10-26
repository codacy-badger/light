#pragma once

#include "parser/pipe/symbol_resolution.hpp"

void Symbol_Resolution::on_statement(Ast_Statement* stm) {
    set<const char*, cmp_str> unresolved_symbols;
    if (!check_symbols(stm, &unresolved_symbols)) {
        for (auto symbol : unresolved_symbols) {
			this->unresolved_symbols[symbol].insert(stm);
        }
    } else this->to_next(stm);
}

bool Symbol_Resolution::check_symbols (Ast_Statement* stm, set<const char*, cmp_str>* sym) {
    switch (stm->stm_type) {
        case AST_STATEMENT_DECLARATION:
            return check_symbols(static_cast<Ast_Declaration*>(stm), sym);
        case AST_STATEMENT_BLOCK:
            return check_symbols(static_cast<Ast_Block*>(stm), sym);
        default: return false;
    }
}

bool Symbol_Resolution::check_symbols (Ast_Declaration* decl, set<const char*, cmp_str>* sym) {
    bool result = true;
    if (decl->type)         result &= check_symbols(decl->type, sym);
    if (decl->expression)   result &= check_symbols(decl->expression, sym);

	if (result) {
		auto it1 = this->unresolved_type_defn_references.find(decl->name);
		if (it1 != this->unresolved_type_defn_references.end()) {
			for (auto ref : this->unresolved_type_defn_references[decl->name]) {
				if (decl->expression->exp_type == AST_EXPRESSION_TYPE_DEFINITION) {
					auto type_defn = static_cast<Ast_Type_Definition*>(decl->expression);
					*ref = type_defn;
				}
			}
			this->unresolved_type_defn_references.erase(decl->name);
		}

		auto it2 = this->unresolved_decl_references.find(decl->name);
		if (it2 != this->unresolved_decl_references.end()) {
			for (auto ref : this->unresolved_decl_references[decl->name]) {
				*ref = decl;
			}
			this->unresolved_decl_references.erase(decl->name);
		}

		auto it3 = this->unresolved_symbols.find(decl->name);
		if (it3 != this->unresolved_symbols.end()) {
			for (auto stm : this->unresolved_symbols[decl->name]) {
				this->on_statement(stm);
			}
			this->unresolved_symbols.erase(decl->name);
		}
	}

    return result;
}

bool Symbol_Resolution::check_symbols (Ast_Block* block, set<const char*, cmp_str>* sym) {
    bool result = true;
    for (auto stm : block->list)
        result &= check_symbols(stm, sym);
    return result;
}

bool Symbol_Resolution::check_symbols (Ast_Expression* exp, set<const char*, cmp_str>* sym) {
    switch (exp->exp_type) {
        case AST_EXPRESSION_FUNCTION:
            return check_symbols(static_cast<Ast_Function*>(exp), sym);
        case AST_EXPRESSION_TYPE_INSTANCE:
            return check_symbols(static_cast<Ast_Type_Instance*>(exp), sym);
        case AST_EXPRESSION_BINARY:
            return check_symbols(static_cast<Ast_Binary*>(exp), sym);
        case AST_EXPRESSION_UNARY:
            return check_symbols(static_cast<Ast_Unary*>(exp), sym);
        case AST_EXPRESSION_IDENT: {
            auto ident = static_cast<Ast_Ident*>(exp);
            if (!ident->declaration) {
				this->unresolved_decl_references[ident->name].insert(&ident->declaration);
                sym->insert(ident->name);
                return false;
            } else return true;
        }
        case AST_EXPRESSION_LITERAL: return true;
        default: return false;
    }
}

bool Symbol_Resolution::check_symbols (Ast_Binary* binary, set<const char*, cmp_str>* sym) {
    return check_symbols(binary->rhs, sym) & check_symbols(binary->lhs, sym);
}

bool Symbol_Resolution::check_symbols (Ast_Unary* unary, set<const char*, cmp_str>* sym) {
    return check_symbols(unary->exp, sym);
}

bool Symbol_Resolution::check_symbols (Ast_Function* fn, set<const char*, cmp_str>* sym) {
    return check_symbols(fn->type, sym) & check_symbols(fn->scope, sym);
}

bool Symbol_Resolution::check_symbols (Ast_Type_Instance* ty_inst, set<const char*, cmp_str>* sym) {
    switch (ty_inst->type_inst_type) {
        case AST_TYPE_INST_NAMED: {
            auto named_type = static_cast<Ast_Named_Type*>(ty_inst);
            if (!named_type->definition) {
				this->unresolved_type_defn_references[named_type->name].insert(&named_type->definition);
                sym->insert(named_type->name);
                return false;
            } else return true;
        }
        case AST_TYPE_INST_POINTER:
            return check_symbols(static_cast<Ast_Pointer_Type*>(ty_inst), sym);
        case AST_TYPE_INST_FUNCTION:
            return check_symbols(static_cast<Ast_Function_Type*>(ty_inst), sym);
        default: return false;
    }
}

bool Symbol_Resolution::check_symbols (Ast_Pointer_Type* ptr_type, set<const char*, cmp_str>* sym) {
    return check_symbols(ptr_type->base, sym);
}

bool Symbol_Resolution::check_symbols (Ast_Function_Type* fn_type, set<const char*, cmp_str>* sym) {
    bool result = true;
    result &= check_symbols(fn_type->return_type, sym);
    for (auto param_type : fn_type->parameters)
        result &= check_symbols(param_type, sym);
    return result;
}
