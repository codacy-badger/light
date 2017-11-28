#include "parser/pipe/unique_types.hpp"

void Unique_Types::on_statement(Ast_Statement* stm) {
    this->unique(stm);
    this->to_next(stm);
}

void Unique_Types::unique (Ast_Statement* stm) {
    switch (stm->stm_type) {
        case AST_STATEMENT_BLOCK: {
            auto block = static_cast<Ast_Block*>(stm);
            for (auto _stm : block->list)
                this->unique(_stm);
            break;
        }
        case AST_STATEMENT_DECLARATION: {
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (decl->type) this->unique(&decl->type);
			if (decl->expression) this->unique(&decl->expression);
			break;
        }
        case AST_STATEMENT_IF: {
            auto _if = static_cast<Ast_If*>(stm);
            this->unique(&_if->condition);
            this->unique(_if->then_statement);
            if (_if->else_statement) this->unique(_if->else_statement);
			break;
        }
        case AST_STATEMENT_WHILE: {
            auto _while = static_cast<Ast_While*>(stm);
            this->unique(&_while->condition);
            this->unique(_while->statement);
			break;
        }
        case AST_STATEMENT_RETURN: {
            auto ret = static_cast<Ast_Return*>(stm);
            this->unique(&ret->exp);
			break;
        }
        case AST_STATEMENT_EXPRESSION: {
            auto exp = static_cast<Ast_Expression*>(stm);
            this->unique(&exp);
			break;
        }
        default: break;
    }
}

void Unique_Types::unique (Ast_Expression** exp) {
    switch ((*exp)->exp_type) {
        case AST_EXPRESSION_CAST: {
            auto cast = reinterpret_cast<Ast_Cast*>(*exp);
            this->unique(&cast->cast_to);
            break;
        }
        case AST_EXPRESSION_FUNCTION: {
            auto func = static_cast<Ast_Function*>(*exp);
            this->unique(&func->type);
			this->unique(func->scope);
            break;
        }
		case AST_EXPRESSION_TYPE_DEFINITION: {
			this->unique(reinterpret_cast<Ast_Type_Definition**>(exp));
			break;
		}
		case AST_EXPRESSION_BINARY: {
            auto binary = static_cast<Ast_Binary*>(*exp);
			this->unique(&binary->lhs);
			this->unique(&binary->rhs);
			break;
		}
		case AST_EXPRESSION_UNARY: {
            auto unary = static_cast<Ast_Unary*>(*exp);
			this->unique(&unary->exp);
			break;
		}
        default: break;
    }
}

void Unique_Types::unique (Ast_Type_Definition** type_def) {
    switch ((*type_def)->typedef_type) {
        case AST_TYPEDEF_STRUCT: {
            auto str_type = static_cast<Ast_Struct_Type*>(*type_def);
            for (int i = 0; i < str_type->attributes.size(); i++) {
        		this->unique(str_type->attributes[i]);
        	}
            break;
        }
        case AST_TYPEDEF_POINTER: {
            this->unique(reinterpret_cast<Ast_Pointer_Type**>(type_def));
            break;
        }
        case AST_TYPEDEF_FUNCTION: {
            this->unique(reinterpret_cast<Ast_Function_Type**>(type_def));
            break;
        }
        default: break;
    }
}

void Unique_Types::unique (Ast_Pointer_Type** ptr_type) {
    this->unique(&(*ptr_type)->base);

    auto it = this->ptr_types.find((*ptr_type)->base);
    if (it != this->ptr_types.end()) {
		if (*ptr_type != this->ptr_types[(*ptr_type)->base]) {
			delete *ptr_type;
			(*ptr_type) = this->ptr_types[(*ptr_type)->base];
		}
    } else {
        this->ptr_types[(*ptr_type)->base] = (*ptr_type);
    }

	if ((*ptr_type)->name == NULL) {
		auto base_type_def = static_cast<Ast_Type_Definition*>((*ptr_type)->base);
		auto base_name_length = strlen(base_type_def->name);
		(*ptr_type)->name = (char*) malloc(base_name_length + 2);
		(*ptr_type)->name[0] = '*';
		strcpy((*ptr_type)->name + 1, base_type_def->name);
		(*ptr_type)->name[base_name_length + 2] = '\0';
	}
}

bool func_type_are_equal (Ast_Function_Type* func_type1, Ast_Function_Type* func_type2) {
    if (func_type1->parameter_decls.size() != func_type2->parameter_decls.size()) return false;
    if (func_type1->return_type != func_type2->return_type) return false;
    for (int i = 0; i < func_type1->parameter_decls.size(); i++) {
        auto decl_1 = func_type1->parameter_decls[i];
        auto decl_2 = func_type2->parameter_decls[i];
        // since we've already "uniqued" the parameter types, we can
        // check if they're the same using pointers
        if (decl_1->type != decl_2->type) return false;
    }
    return true;
}

void Unique_Types::unique (Ast_Function_Type** func_type) {
    for (int i = 0; i < (*func_type)->parameter_decls.size(); i++) {
        this->unique((*func_type)->parameter_decls[i]);
    }
    this->unique(&(*func_type)->return_type);

    for (auto _func_type : this->func_types) {
        if (func_type_are_equal(*func_type, _func_type)) {
			if (*func_type != _func_type) {
				delete *func_type;
				(*func_type) = _func_type;
			}
			return;
        }
    }
    this->func_types.push_back(*func_type);

	if ((*func_type)->name == NULL) {
		auto par_decls = (*func_type)->parameter_decls;

		size_t name_size = strlen("fn (");
		if (par_decls.size() > 0) {
			auto type_def = static_cast<Ast_Type_Definition*>(par_decls[0]->type);
			name_size += strlen(type_def->name);
			for (int i = 1; i < par_decls.size(); i++) {
				name_size += strlen(", ");
				auto type_def = static_cast<Ast_Type_Definition*>(par_decls[i]->type);
				name_size += strlen(type_def->name);
			}
		}
		name_size += strlen(") -> ");
		auto type_def = static_cast<Ast_Type_Definition*>((*func_type)->return_type);
		name_size += strlen(type_def->name);
		(*func_type)->name = (char*) malloc(name_size + 1);

		size_t offset = 0;
		strcpy((*func_type)->name, "fn (");
		offset += strlen("fn (");

		if (par_decls.size() > 0) {
			auto type_def = static_cast<Ast_Type_Definition*>(par_decls[0]->type);
			strcpy((*func_type)->name + offset, type_def->name);
			offset += strlen(type_def->name);
			for (int i = 1; i < par_decls.size(); i++) {
				strcpy((*func_type)->name + offset, ", ");
				offset += strlen(", ");
				strcpy((*func_type)->name + offset, type_def->name);
				offset += strlen(type_def->name);
			}
		}

		strcpy((*func_type)->name + offset, ") -> ");
		offset += strlen(") -> ");
		type_def = static_cast<Ast_Type_Definition*>((*func_type)->return_type);
		strcpy((*func_type)->name + offset, type_def->name);
		offset += strlen(type_def->name);
		(*func_type)->name[offset] = '\0';
	}
}
