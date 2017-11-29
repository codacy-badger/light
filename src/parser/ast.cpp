#pragma once

#include "parser/ast.hpp"

#include <string.h>

void ast_copy_location_info (Ast* destination, Ast* source) {
	destination->filename = source->filename;
	destination->line = source->line;
	destination->col = source->col;
}

void Ast_Block::find_declarations (std::vector<Ast_Declaration*>* decls, bool recurse) {
	if (this->scope_of) {
		for (auto decl : this->scope_of->type->parameter_decls) {
			decls->push_back(decl);
		}
	}
    for (auto stm : this->list) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION)
            decls->push_back(static_cast<Ast_Declaration*>(stm));
    }
}

Ast_Declaration* Ast_Block::find_declaration (const char* name, bool recurse) {
	if (this->scope_of) {
		for (auto decl : this->scope_of->type->parameter_decls) {
			if (strcmp(decl->name, name) == 0) return decl;
		}
	}
    for (auto stm : this->list) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION) {
            auto decl = static_cast<Ast_Declaration*>(stm);
            if (strcmp(decl->name, name) == 0) return decl;
        }
    }
    if (recurse && this->parent)
        return this->parent->find_declaration(name, recurse);
    else return NULL;
}

Ast_Type_Definition* Ast_Block::find_type_definition (const char* name, bool recurse) {
    auto decl = this->find_declaration(name, recurse);
    if (decl && decl->expression->exp_type == AST_EXPRESSION_TYPE_DEFINITION)
        return static_cast<Ast_Type_Definition*>(decl->expression);
    else return NULL;
}

Ast_Function* Ast_Block::find_function (bool recurse) {
	if (this->scope_of) {
		return this->scope_of;
	} else if (recurse && this->parent) {
		return this->parent->find_function(recurse);
	} else return NULL;
}

void Ast_Binary::setOP (Token_Type tType) {
	this->binary_op = this->typeToOP(tType);
}

Ast_Binary_Type Ast_Binary::typeToOP (Token_Type tType) {
	switch (tType) {
		case TOKEN_EQUAL: 			return AST_BINARY_ASSIGN;
		case TOKEN_DOT: 			return AST_BINARY_ATTRIBUTE;

		case TOKEN_DOUBLE_AMP:		return AST_BINARY_LOGICAL_AND;
		case TOKEN_DOUBLE_PIPE:		return AST_BINARY_LOGICAL_OR;

		case TOKEN_ADD: 			return AST_BINARY_ADD;
		case TOKEN_SUB: 			return AST_BINARY_SUB;
		case TOKEN_MUL: 			return AST_BINARY_MUL;
		case TOKEN_DIV: 			return AST_BINARY_DIV;

		case TOKEN_AMP:				return AST_BINARY_BITWISE_AND;
		case TOKEN_PIPE:			return AST_BINARY_BITWISE_OR;
		case TOKEN_HAT:				return AST_BINARY_BITWISE_XOR;
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
		case TOKEN_DOT:   			return 1;

		case TOKEN_DOUBLE_AMP:		return 1;
		case TOKEN_DOUBLE_PIPE:		return 1;
		case TOKEN_EXCLAMATION:		return 1;

		case TOKEN_DOUBLE_ADD: 		return 2;
		case TOKEN_DOUBLE_SUB: 		return 2;
		case TOKEN_ADD:   			return 2;
		case TOKEN_SUB:   			return 2;
		case TOKEN_MUL:   			return 3;
		case TOKEN_DIV:   			return 3;

		case TOKEN_AMP:				return 3;
		case TOKEN_PIPE:			return 3;
		case TOKEN_HAT:				return 3;
		case TOKEN_TILDE:			return 3;
		case TOKEN_RIGHT_SHIFT:		return 3;
		case TOKEN_LEFT_SHIFT:		return 3;

		case TOKEN_DOUBLE_EQUAL:	return 4;
		case TOKEN_NOT_EQUAL:		return 4;
		case TOKEN_GREATER_EQUAL:	return 4;
		case TOKEN_LESSER_EQUAL:	return 4;
		case TOKEN_GREATER:			return 4;
		case TOKEN_LESSER:			return 4;

		default: 		  			return 0;
	}
}

bool Ast_Binary::getLeftAssociativity (Token_Type opToken) {
	switch (opToken) {
		case TOKEN_EQUAL: 			return false;
		case TOKEN_DOT:   			return false;

		case TOKEN_DOUBLE_AMP:		return false;
		case TOKEN_DOUBLE_PIPE:		return false;
		case TOKEN_EXCLAMATION:		return false;

		case TOKEN_DOUBLE_ADD: 		return false;
		case TOKEN_DOUBLE_SUB: 		return false;
		case TOKEN_ADD:   			return false;
		case TOKEN_SUB:   			return false;
		case TOKEN_MUL:   			return false;
		case TOKEN_DIV:   			return false;

		case TOKEN_AMP:				return false;
		case TOKEN_PIPE:			return false;
		case TOKEN_HAT:				return false;
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
		case TOKEN_SUB: return AST_UNARY_NEGATE;
		case TOKEN_MUL: return AST_UNARY_REFERENCE;
		case TOKEN_AMP: return AST_UNARY_DEREFERENCE;
		default: 		return AST_UNARY_UNINITIALIZED;
	};
}

bool Ast_Ident::operator ==(const Ast_Ident* other) const {
	return strcmp(this->name, other->name) == 0;
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
