#include "parser/pipe/symbol_resolution.hpp"

#include "compiler.hpp"

void try_replace_ident_by_const (Ast_Ident** ident_ptr2) {
    auto decl = reinterpret_cast<Ast_Declaration*>((*ident_ptr2)->declaration);
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

void Symbol_Resolution::on_statement(Ast_Statement* stm) {
    vector<Ast_Ident**> idents;
    check_symbols(stm, &idents);

	if (stm->stm_type == AST_STATEMENT_DECLARATION) {
		auto decl = static_cast<Ast_Declaration*>(stm);
		auto it = idents.begin();
		while (it != idents.end()) {
            if (try_resolve(*it, decl)) {
                it = idents.erase(it);
            } else it++;
		}
	}

    if (idents.size() > 0) {
        this->unresolved[stm] = idents;
    } else this->on_resolved(stm);
}

void Symbol_Resolution::on_finish () {
    bool have_unresolved = false;
    if (this->unresolved.size() > 0) {
		for (auto dependencies : this->unresolved) {
			for (auto ident_ptr2 : dependencies.second) {
                Light_Compiler::inst->error((*ident_ptr2), "Unresolved symbol: '%s'", (*ident_ptr2)->name);
                have_unresolved = true;
			}
        }
		if (have_unresolved) {
            Light_Compiler::inst->stop();
            return;
        }
    }
    this->try_finish();
}

size_t Symbol_Resolution::on_resolved (Ast_Statement* stm) {
    this->to_next(stm);
    size_t count = 0;
    if (stm->stm_type == AST_STATEMENT_DECLARATION) {
        auto decl = static_cast<Ast_Declaration*>(stm);
		if (decl->is_constant()) {
            auto stm_idents = this->unresolved.begin();
            while (stm_idents != this->unresolved.end()) {
                auto ident_ptr2 = stm_idents->second.begin();
                while (ident_ptr2 != stm_idents->second.end()) {
                    if (try_resolve(*ident_ptr2, decl)) {
                        ident_ptr2 = stm_idents->second.erase(ident_ptr2);
                    } else ident_ptr2++;
                }

                if (stm_idents->second.size() == 0) {
                    auto _stm = stm_idents->first;
                    stm_idents = this->unresolved.erase(stm_idents);

                    auto resolved_count = this->on_resolved(_stm);
                    for (int i = 0; i < resolved_count; i++) stm_idents++;
                    count += resolved_count + 1;
                } else stm_idents++;
            }
		}
    }
    return count;
}

bool Symbol_Resolution::is_unresolved (const char* name) {
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

void Symbol_Resolution::check_symbols (Ast_Statement* stm, vector<Ast_Ident**>* sym) {
	for (auto note : stm->notes) {
		if (note->arguments) {
            check_symbols(&note->arguments, sym);
        }
	}
    switch (stm->stm_type) {
        case AST_STATEMENT_DECLARATION: {
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (decl->type)         check_symbols(&decl->type, sym);
            if (decl->expression)   check_symbols(&decl->expression, sym);
			break;
        }
        case AST_STATEMENT_BLOCK: {
            auto block = static_cast<Ast_Block*>(stm);
            for (auto _stm : block->list) {
                check_symbols(_stm, sym);
            }
			break;
        }
        case AST_STATEMENT_IF: {
            auto _if = static_cast<Ast_If*>(stm);
        	check_symbols(&_if->condition, sym);
        	check_symbols(_if->then_statement, sym);
        	if (_if->else_statement) check_symbols(_if->else_statement, sym);
			break;
        }
        case AST_STATEMENT_WHILE: {
            auto _while = static_cast<Ast_While*>(stm);
        	check_symbols(&_while->condition, sym);
        	check_symbols(_while->statement, sym);
			break;
        }
        case AST_STATEMENT_RETURN: {
            auto ret = static_cast<Ast_Return*>(stm);
            if (ret->exp) check_symbols(&ret->exp, sym);
			break;
        }
        case AST_STATEMENT_EXPRESSION: {
            auto exp = static_cast<Ast_Expression*>(stm);
            check_symbols(&exp, sym);
			break;
        }
        default: break;
    }
}

void Symbol_Resolution::check_symbols (Ast_Expression** exp, vector<Ast_Ident**>* sym) {
    switch ((*exp)->exp_type) {
        case AST_EXPRESSION_FUNCTION: {
            auto func = reinterpret_cast<Ast_Function**>(exp);
            check_symbols(&(*func)->type, sym);
            check_symbols((*func)->scope, sym);
			break;
        }
		case AST_EXPRESSION_TYPE_DEFINITION: {
			check_symbols(reinterpret_cast<Ast_Type_Definition**>(exp), sym);
			break;
        }
		case AST_EXPRESSION_COMMA_SEPARATED_ARGUMENTS: {
            auto args = reinterpret_cast<Ast_Comma_Separated_Arguments**>(exp);
            for (int i = 0; i < (*args)->values.size(); i++) {
        		check_symbols(&(*args)->values[i], sym);
        	}
			break;
        }
		case AST_EXPRESSION_CALL: {
			check_symbols(reinterpret_cast<Ast_Function_Call**>(exp), sym);
			break;
        }
		case AST_EXPRESSION_CAST: {
            auto cast = reinterpret_cast<Ast_Cast**>(exp);
        	check_symbols(&(*cast)->value, sym);
        	check_symbols(&(*cast)->cast_to, sym);
			break;
        }
        case AST_EXPRESSION_BINARY: {
            auto binary = reinterpret_cast<Ast_Binary**>(exp);
            check_symbols(&(*binary)->lhs, sym);
            if ((*binary)->binary_op != AST_BINARY_ATTRIBUTE) {
        		// We don't resolve symbols for struct attributes attributes here,
        		// since we still don't know the type of lhs. GOTO: type_checking
                check_symbols(&(*binary)->rhs, sym);
            }
			break;
        }
        case AST_EXPRESSION_UNARY: {
            auto unary = reinterpret_cast<Ast_Unary**>(exp);
            check_symbols(&(*unary)->exp, sym);
			break;
        }
        case AST_EXPRESSION_IDENT: {
            auto ident_ptr2 = reinterpret_cast<Ast_Ident**>(exp);
            auto ident = (*ident_ptr2);

            if (!ident->declaration) {
                ident->declaration = ident->scope->find_const_declaration(ident->name);
                if (ident->declaration) {
                    if (this->is_unresolved(ident->name)) {
                        sym->push_back(ident_ptr2);
                    } else {
                        try_replace_ident_by_const(ident_ptr2);
                    }
                } else sym->push_back(ident_ptr2);
            } else try_replace_ident_by_const(ident_ptr2);
			break;
        }
        case AST_EXPRESSION_LITERAL: break;
        default: break;
    }
}

void Symbol_Resolution::check_symbols (Ast_Type_Definition** tydef, vector<Ast_Ident**>* sym) {
	switch ((*tydef)->typedef_type) {
        case AST_TYPEDEF_STRUCT: {
            auto _struct = reinterpret_cast<Ast_Struct_Type**>(tydef);
            for (auto decl : (*_struct)->attributes) {
        		check_symbols(decl, sym);
        	}
			break;
        }
        case AST_TYPEDEF_POINTER: {
            auto ptr_type = reinterpret_cast<Ast_Pointer_Type**>(tydef);
            check_symbols(&(*ptr_type)->base, sym);
			break;
		}
        case AST_TYPEDEF_FUNCTION: {
            auto fn_type = reinterpret_cast<Ast_Function_Type**>(tydef);
            check_symbols(&(*fn_type)->return_type, sym);
        	for (int i = 0; i < (*fn_type)->parameter_decls.size(); i++) {
        		check_symbols((*fn_type)->parameter_decls[i], sym);
        	}
			break;
		}
        case AST_TYPEDEF_ARRAY: {
            auto arr_type = reinterpret_cast<Ast_Array_Type**>(tydef);
        	check_symbols(&(*arr_type)->base, sym);
            if ((*arr_type)->count) check_symbols(&(*arr_type)->count, sym);
			break;
		}
        default: break;
    }
}

void Symbol_Resolution::check_symbols (Ast_Function_Type** fn_type, vector<Ast_Ident**>* sym) {
    check_symbols(&(*fn_type)->return_type, sym);
	for (int i = 0; i < (*fn_type)->parameter_decls.size(); i++) {
		check_symbols((*fn_type)->parameter_decls[i], sym);
	}
}

void Symbol_Resolution::check_symbols (Ast_Comma_Separated_Arguments** args, vector<Ast_Ident**>* sym) {
	for (int i = 0; i < (*args)->values.size(); i++) {
		check_symbols(&(*args)->values[i], sym);
	}
}

void Symbol_Resolution::check_symbols (Ast_Function_Call** call, vector<Ast_Ident**>* sym) {
	if ((*call)->fn->exp_type != AST_EXPRESSION_FUNCTION) {
        check_symbols(&(*call)->fn, sym);
	}
	if ((*call)->args) {
		for (int i = 0; i < (*call)->args->values.size(); i++) {
			check_symbols(&(*call)->args->values[i], sym);
		}
	}
}
