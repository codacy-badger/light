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
   if (this->count && this->count->exp_type == AST_EXPRESSION_LITERAL) {
	   auto lit = static_cast<Ast_Literal*>(this->count);
	   return lit->uint_value;
   } else return 0;
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

Ast_Struct_Type* ast_make_slice_type (Ast_Expression* base_type, Ast_Struct_Type* size_type) {
	auto slice_type = new Ast_Struct_Type();
	slice_type->is_slice = true;

	auto length_attr = new Ast_Declaration();
	length_attr->name = "length";
	length_attr->type = size_type ? size_type : g_compiler->type_def_u64;
	slice_type->attributes.push_back(length_attr);

	auto data_attr = new Ast_Declaration();
	data_attr->name = "data";
	data_attr->type = g_compiler->types->get_or_create_pointer_type(base_type);
	slice_type->attributes.push_back(data_attr);

	return slice_type;
}

Ast_Declaration* ast_make_declaration (const char* name, Ast_Expression* exp, bool is_const) {
    auto decl = new Ast_Declaration();
	if (is_const) decl->decl_flags |= AST_DECL_FLAG_CONSTANT;
    decl->expression = exp;
    decl->name = name;
    return decl;
}
