#include "parser/ast.hpp"

#include <string.h>

void ast_copy_location_info (Ast* destination, Ast* source) {
	destination->filename = source->filename;
	destination->line = source->line;
	destination->col = source->col;
}

Ast_Declaration* Ast_Block::find_declaration (const char* _name, size_t max_line, bool recurse, bool is_out_scope) {
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
						if (decl->line <= max_line) return decl;
					} else return decl;
				}
			}
        }
    }
	if (this->scope_of) {
		for (auto decl : this->scope_of->type->parameter_decls) {
			if (strcmp(decl->name, _name) == 0) {
				if (is_out_scope) {
					if (decl->is_constant() || decl->is_global()) return decl;
				} else return decl;
			}
		}
		is_out_scope = true;
	}
    if (recurse && this->parent) {
        return this->parent->find_declaration(_name, max_line, recurse, is_out_scope);
    } else return NULL;
}

Ast_Function* Ast_Block::get_function () {
	if (this->scope_of) return this->scope_of;
	else {
		Ast_Block* block = this;
		while (block->parent) {
			block = this->parent;
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

void Ast_Binary::setOP (Token_Type tType) {
	this->binary_op = this->typeToOP(tType);
}

Ast_Binary_Type Ast_Binary::typeToOP (Token_Type tType) {
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

short Ast_Binary::getPrecedence (Token_Type opToken) {
	switch (opToken) {
		case TOKEN_EQUAL: 			return 1;
		case TOKEN_SQ_BRAC_OPEN:	return 2;

		case TOKEN_DOUBLE_AMP:		return 2;
		case TOKEN_DOUBLE_PIPE:		return 2;
		case TOKEN_EXCLAMATION:		return 2;

		case TOKEN_DOUBLE_ADD: 		return 3;
		case TOKEN_DOUBLE_SUB: 		return 3;
		case TOKEN_ADD:   			return 3;
		case TOKEN_SUB:   			return 3;
		case TOKEN_MUL:   			return 4;
		case TOKEN_DIV:   			return 4;
		case TOKEN_PERCENT:			return 4;

		case TOKEN_AMP:				return 4;
		case TOKEN_PIPE:			return 4;
		case TOKEN_CARET:			return 4;
		case TOKEN_TILDE:			return 4;
		case TOKEN_RIGHT_SHIFT:		return 4;
		case TOKEN_LEFT_SHIFT:		return 4;

		case TOKEN_DOUBLE_EQUAL:	return 5;
		case TOKEN_NOT_EQUAL:		return 5;
		case TOKEN_GREATER_EQUAL:	return 5;
		case TOKEN_LESSER_EQUAL:	return 5;
		case TOKEN_GREATER:			return 5;
		case TOKEN_LESSER:			return 5;

		case TOKEN_DOT:   			return 8;

		default: 		  			return 0;
	}
}

bool Ast_Binary::getLeftAssociativity (Token_Type opToken) {
	switch (opToken) {
		case TOKEN_EQUAL: 			return false;
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

void Ast_Unary::setOP (Token_Type tType) {
	this->unary_op = this->typeToOP(tType);
}

Ast_Unary_Type Ast_Unary::typeToOP (Token_Type tType) {
	switch (tType) {
		case TOKEN_AMP: 		return AST_UNARY_DEREFERENCE;
		case TOKEN_MUL: 		return AST_UNARY_REFERENCE;
		case TOKEN_SUB: 		return AST_UNARY_NEGATE;
		case TOKEN_EXCLAMATION: return AST_UNARY_NOT;
		default: 				return AST_UNARY_UNINITIALIZED;
	};
}

bool Ast_Ident::operator ==(const Ast_Ident* other) const {
	return strcmp(this->name, other->name) == 0;
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

Ast_Declaration* ast_make_declaration (const char* name, Ast_Expression* exp, bool is_const) {
    auto decl = new Ast_Declaration();
	if (is_const) decl->decl_flags |= AST_DECL_FLAG_CONSTANT;
    decl->expression = exp;
    decl->name = name;
    return decl;
}
