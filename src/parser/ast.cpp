#include "parser/ast.hpp"

#include "compiler.hpp"

#include <string.h>

Ast_Note* Ast_Statement::get_note (const char* name) {
    for (auto note : this->notes) {
        if (strcmp(note->name, name) == 0) return note;
    }
    return NULL;
}

Ast_Note* Ast_Statement::remove_note (const char* name) {
    auto it = this->notes.begin();
    while (it != this->notes.end()) {
        if (strcmp((*it)->name, name) == 0) {
            auto note = *it;
            this->notes.erase(it);
            return note;
        }
        it++;
    }
    return NULL;
}

Ast_Declaration* Ast_Block::find_declaration (const char* _name, bool recurse, bool is_out_scope) {
    for (auto stm : this->list) {
		// TODO: check why the F. do I have to check for null here
		// there should never be null values inside this.
		// not checking crashes in Unix build
        if (stm && stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (strcmp(decl->name, _name) == 0) {
				if (is_out_scope) {
					if (decl->is_constant() || decl->is_global()) return decl;
				} else {
					if (!decl->is_constant() && !decl->is_global()) {
						return decl;
					} else return decl;
				}
			}
        }
    }
	if (this->scope_of) {
		for (auto decl : this->scope_of->arg_decls) {
			if (strcmp(decl->name, _name) == 0) {
				if (is_out_scope) {
					if (decl->is_constant() || decl->is_global()) return decl;
				} else return decl;
			}
		}
		is_out_scope = true;
	}
    if (recurse && this->parent) {
        return this->parent->find_declaration(_name, recurse, is_out_scope);
    } else return NULL;
}

Ast_Declaration* Ast_Block::find_non_const_declaration (const char* _name) {
    for (auto stm : this->list) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (!decl->is_constant() && strcmp(decl->name, _name) == 0) {
				return decl;
			}
        }
    }
	if (this->scope_of) {
		for (auto decl : this->scope_of->arg_decls) {
			if (strcmp(decl->name, _name) == 0) {
				return decl;
			}
		}
		if (!this->parent->is_global) return NULL;
	}
    if (this->parent) {
        return this->parent->find_non_const_declaration(_name);
    } else return NULL;
}

Ast_Declaration* Ast_Block::find_const_declaration (const char* _name) {
    for (auto stm : this->list) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (decl->is_constant() && strcmp(decl->name, _name) == 0) {
				return decl;
			}
        }
    }
    if (this->parent) {
        return this->parent->find_const_declaration(_name);
    } else return NULL;
}

bool Ast_Block::is_ancestor (Ast_Block* other) {
	if (this == other) return true;
	else {
		Ast_Block* block = this;
		while (block->parent) {
			block = block->parent;
			if (block == other) return true;
		}
		return false;
	}
}

Ast_Function* Ast_Block::get_function () {
	if (this->scope_of) return this->scope_of;
	else {
		Ast_Block* block = this;
		while (block->parent) {
			block = block->parent;
			if (block->scope_of) return block->scope_of;
		}
		return NULL;
	}
}

Ast_Declaration* Ast_Struct_Type::find_attribute (const char* _name) {
	for (auto decl : this->attributes) {
		if (strcmp(decl->name, _name) == 0) return decl;
	}
	return NULL;
}

uint64_t Ast_Array_Type::length () {
	if (!this->_length) {
		assert(this->length_exp);
		assert(this->length_exp->exp_type == AST_EXPRESSION_LITERAL);
		this->_length = static_cast<Ast_Literal*>(this->length_exp)->uint_value;
	}
	return this->_length;
}

Ast_Slice_Type::Ast_Slice_Type(Ast_Expression* base_type) {
	this->typedef_type = AST_TYPEDEF_STRUCT;
	this->base = base_type;
	this->is_slice = true;

	auto length_attr = new Ast_Declaration();
	length_attr->name = "length";
	length_attr->type = g_compiler->type_def_u64;
	this->attributes.push_back(length_attr);

	auto data_attr = new Ast_Declaration();
	data_attr->name = "data";
	data_attr->type = g_compiler->types->get_or_create_pointer_type(base_type);
	this->attributes.push_back(data_attr);
}

Ast_Binary_Type token_to_binop (Token_Type tType) {
	switch (tType) {
		case TOKEN_EQUAL: 			return AST_BINARY_ASSIGN;
		case TOKEN_DOT: 			return AST_BINARY_ATTRIBUTE;
		case TOKEN_SQ_BRAC_OPEN: 	return AST_BINARY_SUBSCRIPT;

		case TOKEN_DOUBLE_AMP:		return AST_BINARY_LOGICAL_AND;
		case TOKEN_DOUBLE_PIPE:		return AST_BINARY_LOGICAL_OR;

		case TOKEN_ADD: 			return AST_BINARY_ADD;
		case TOKEN_SUB: 			return AST_BINARY_SUB;
		case TOKEN_MUL: 			return AST_BINARY_MUL;
		case TOKEN_DIV: 			return AST_BINARY_DIV;
		case TOKEN_PERCENT:			return AST_BINARY_REM;

		case TOKEN_AMP:				return AST_BINARY_BITWISE_AND;
		case TOKEN_PIPE:			return AST_BINARY_BITWISE_OR;
		case TOKEN_CARET:				return AST_BINARY_BITWISE_XOR;
		case TOKEN_RIGHT_SHIFT:		return AST_BINARY_BITWISE_RIGHT_SHIFT;
		case TOKEN_LEFT_SHIFT:		return AST_BINARY_BITWISE_LEFT_SHIFT;

		case TOKEN_DOUBLE_EQUAL:	return AST_BINARY_EQ;
		case TOKEN_NOT_EQUAL:		return AST_BINARY_NEQ;
		case TOKEN_GREATER_EQUAL:	return AST_BINARY_GTE;
		case TOKEN_LESSER_EQUAL:	return AST_BINARY_LTE;
		case TOKEN_GREATER:			return AST_BINARY_GT;
		case TOKEN_LESSER:			return AST_BINARY_LT;
		default: 					return AST_BINARY_UNINITIALIZED;
	};
}

short Ast_Binary::get_precedence (Token_Type opToken) {
	switch (opToken) {
		default: 		  			return 0;
		case TOKEN_EQUAL: 			return 1;
		case TOKEN_SQ_BRAC_OPEN:	return 2;
		case TOKEN_DOUBLE_PIPE:		return 3;
		case TOKEN_DOUBLE_AMP:		return 4;
		case TOKEN_PIPE:			return 5;
		case TOKEN_TILDE:			return 6;
		case TOKEN_AMP:				return 7;

		case TOKEN_DOUBLE_EQUAL:
		case TOKEN_NOT_EQUAL:		return 8;

		case TOKEN_GREATER_EQUAL:
		case TOKEN_LESSER_EQUAL:
		case TOKEN_GREATER:
		case TOKEN_LESSER:			return 9;

		case TOKEN_RIGHT_SHIFT:
		case TOKEN_LEFT_SHIFT:		return 10;

		case TOKEN_ADD:
		case TOKEN_SUB:   			return 11;

		case TOKEN_MUL:
		case TOKEN_DIV:
		case TOKEN_PERCENT:			return 12;

		case TOKEN_CARET:
		case TOKEN_EXCLAMATION:		return 13;

		case TOKEN_DOUBLE_ADD:
		case TOKEN_DOUBLE_SUB:
		case TOKEN_DOT:   			return 14;

	}
}

bool Ast_Binary::is_left_associative (Token_Type opToken) {
	switch (opToken) {
		case TOKEN_EQUAL: 			return true;
		case TOKEN_DOT:   			return false;
		case TOKEN_SQ_BRAC_OPEN:	return false;

		case TOKEN_DOUBLE_AMP:		return false;
		case TOKEN_DOUBLE_PIPE:		return false;
		case TOKEN_EXCLAMATION:		return false;

		case TOKEN_DOUBLE_ADD: 		return false;
		case TOKEN_DOUBLE_SUB: 		return false;
		case TOKEN_ADD:   			return false;
		case TOKEN_SUB:   			return false;
		case TOKEN_MUL:   			return false;
		case TOKEN_DIV:   			return false;
		case TOKEN_PERCENT:   		return false;

		case TOKEN_AMP:				return false;
		case TOKEN_PIPE:			return false;
		case TOKEN_CARET:			return false;
		case TOKEN_TILDE:			return false;
		case TOKEN_RIGHT_SHIFT:		return false;
		case TOKEN_LEFT_SHIFT:		return false;

		case TOKEN_DOUBLE_EQUAL:	return false;
		case TOKEN_NOT_EQUAL:		return false;
		case TOKEN_GREATER_EQUAL:	return false;
		case TOKEN_LESSER_EQUAL:	return false;
		case TOKEN_GREATER:			return false;
		case TOKEN_LESSER:			return false;
		default: 		  			return false;
	}
}

char* _get_type_name (Ast_Expression* exp) {
	if (exp->exp_type == AST_EXPRESSION_IDENT) {
		auto ident = static_cast<Ast_Ident*>(exp);
		return const_cast<char*>(ident->name);
	} else if (exp->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
		auto type_inst = static_cast<Ast_Type_Instance*>(exp);
		ast_compute_type_name_if_needed(type_inst);
		return type_inst->name;
	} else abort();
}

void ast_compute_type_name_if_needed (Ast_Type_Instance* type_inst) {
	if (!type_inst->name) {
		switch (type_inst->typedef_type) {
	        case AST_TYPEDEF_STRUCT: {
				auto _struct = static_cast<Ast_Struct_Type*>(type_inst);
				if (_struct->is_slice) {
					auto slice = static_cast<Ast_Slice_Type*>(type_inst);
					auto base_name = _get_type_name(slice->base);
	        		auto base_name_length = strlen(base_name);
	        		slice->name = (char*) malloc(base_name_length + 3);
	        		slice->name[0] = '[';
	        		slice->name[1] = ']';
	        		memcpy(slice->name + 2, base_name, base_name_length);
	        		slice->name[base_name_length + 2] = '\0';
				}
				break;
			}
	        case AST_TYPEDEF_POINTER: {
	            auto _ptr = static_cast<Ast_Pointer_Type*>(type_inst);
	            if (_ptr->name == NULL) {
					auto base_name = _get_type_name(_ptr->base);
	        		auto base_name_length = strlen(base_name);
	        		_ptr->name = (char*) malloc(base_name_length + 2);
	        		_ptr->name[0] = '*';
	        		memcpy(_ptr->name + 1,base_name, base_name_length);
	        		_ptr->name[base_name_length + 1] = '\0';
	        	}
	            return;
	        }
	        case AST_TYPEDEF_ARRAY: {
	            auto _arr = static_cast<Ast_Array_Type*>(type_inst);
	            if (_arr->name == NULL) {
					auto base_name = _get_type_name(_arr->base);
	        		auto base_name_length = strlen(base_name);
					_arr->name = (char*) malloc(base_name_length + 23);
					sprintf_s(_arr->name, base_name_length + 23, "[%lld]%s", _arr->length(), base_name);
	        	}
	            return;
	        }
	        case AST_TYPEDEF_FUNCTION: {
	            auto _func = static_cast<Ast_Function_Type*>(type_inst);
	            if (_func->name == NULL) {
	        		auto arg_types = _func->arg_types;

	        		size_t name_size = strlen("fn (");
	        		if (arg_types.size() > 0) {
						auto param_name = _get_type_name(arg_types[0]);
	        			name_size += strlen(param_name);
	        			for (int i = 1; i < arg_types.size(); i++) {
	        				name_size += strlen(", ");
	        				param_name = _get_type_name(arg_types[i]);
	        				name_size += strlen(param_name);
	        			}
	        		}
	        		name_size += strlen(") -> ");
					auto ret_name = _get_type_name(_func->ret_type);
	        		name_size += strlen(ret_name);
	        		_func->name = (char*) malloc(name_size + 1);

	        		size_t offset = 0;
	        		memcpy(_func->name, "fn (", 4);
	        		offset += 4;

					size_t par_type_name_length;
	        		if (arg_types.size() > 0) {
						auto param_name = _get_type_name(arg_types[0]);
						par_type_name_length = strlen(param_name);
	        			memcpy(_func->name + offset, param_name, par_type_name_length);
	        			offset += par_type_name_length;
	        			for (int i = 1; i < arg_types.size(); i++) {
	        				memcpy(_func->name + offset, ", ", 2);
	        				offset += 2;
	        				param_name = _get_type_name(arg_types[i]);
							par_type_name_length = strlen(param_name);
	        				memcpy(_func->name + offset, param_name, par_type_name_length);
	        				offset += par_type_name_length;
	        			}
	        		}

	        		memcpy(_func->name + offset, ") -> ", 5);
	        		offset += 5;
					par_type_name_length = strlen(ret_name);
	        		memcpy(_func->name + offset, ret_name, par_type_name_length);
	        		offset += par_type_name_length;
	        		_func->name[offset] = '\0';
	        	}
	            return;
	        }
	        default: return;
	    }
	}
}

bool ast_type_are_equal (Ast_Type_Instance* type_inst1, Ast_Type_Instance* type_inst2) {
    if (type_inst1 == type_inst2) return true;
    else {
        if (type_inst1->typedef_type != type_inst2->typedef_type) return false;
        switch (type_inst1->typedef_type) {
            case AST_TYPEDEF_STRUCT: return false;
            case AST_TYPEDEF_POINTER: {
                auto ptr_type1 = static_cast<Ast_Pointer_Type*>(type_inst1);
                auto ptr_type2 = static_cast<Ast_Pointer_Type*>(type_inst2);
                auto base_type1 = static_cast<Ast_Type_Instance*>(ptr_type1->base);
                auto base_type2 = static_cast<Ast_Type_Instance*>(ptr_type2->base);
                return ast_type_are_equal(base_type1, base_type2);
                break;
            }
            case AST_TYPEDEF_ARRAY: {
                auto arr_type1 = static_cast<Ast_Array_Type*>(type_inst1);
                auto arr_type2 = static_cast<Ast_Array_Type*>(type_inst2);
                auto base_type1 = static_cast<Ast_Type_Instance*>(arr_type1->base);
                auto base_type2 = static_cast<Ast_Type_Instance*>(arr_type2->base);
                return ast_type_are_equal(base_type1, base_type2);
                break;
            }
            case AST_TYPEDEF_FUNCTION: {
                auto func_type1 = static_cast<Ast_Function_Type*>(type_inst1);
                auto func_type2 = static_cast<Ast_Function_Type*>(type_inst2);
                for (size_t i = 0; i < func_type1->arg_types.size(); i++) {
                    auto arg_type1 = static_cast<Ast_Type_Instance*>(func_type1->arg_types[i]);
                    auto arg_type2 = static_cast<Ast_Type_Instance*>(func_type2->arg_types[i]);
                    if (!ast_type_are_equal(arg_type1, arg_type2)) return false;
                }
                auto ret_type1 = static_cast<Ast_Type_Instance*>(func_type1->ret_type);
                auto ret_type2 = static_cast<Ast_Type_Instance*>(func_type2->ret_type);
                return ast_type_are_equal(ret_type1, ret_type2);
                break;
            }
        }
        return true;
    }
}

Ast_Literal* ast_make_literal (const char* value) {
	auto lit = new Ast_Literal();
	lit->literal_type = AST_LITERAL_STRING;
	lit->string_value = _strdup(value);
	return lit;
}

Ast_Literal* ast_make_literal (unsigned long long value) {
	auto lit = new Ast_Literal();
	lit->literal_type = AST_LITERAL_UNSIGNED_INT;
	lit->uint_value = value;
	return lit;
}

Ast_Ident* ast_make_ident (const char* name) {
	auto out = new Ast_Ident();
	out->name = name;
	return out;
}

Ast_Unary* ast_make_unary (Ast_Unary_Type type, Ast_Expression* expression) {
    return new Ast_Unary(type, expression);
}

Ast_Binary* ast_make_binary (Ast_Binary_Type type, Ast_Expression* lhs, Ast_Expression* rhs) {
    auto binop = new Ast_Binary(type);
    binop->lhs = lhs;
    binop->rhs = rhs;
    return binop;
}

Ast_Declaration* ast_make_declaration (const char* name, Ast_Expression* exp, bool is_const) {
    auto decl = new Ast_Declaration();
	if (is_const) decl->decl_flags |= AST_DECL_FLAG_CONSTANT;
    decl->expression = exp;
    decl->name = name;
    return decl;
}
