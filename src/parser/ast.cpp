#pragma once

#include "parser/ast.hpp"

#include <string.h>

void Ast_Block::find_all_declarations (std::vector<Ast_Declaration*>* decls) {
    for (auto stm : this->list) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION)
            decls->push_back(static_cast<Ast_Declaration*>(stm));
    }
}

Ast_Declaration* Ast_Block::find_declaration (const char* name, bool recurse) {
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

void Ast_Binary::setOP (Token_Type tType) {
	this->binary_op = this->typeToOP(tType);
}

Ast_Binary_Type Ast_Binary::typeToOP (Token_Type tType) {
	switch (tType) {
		case TOKEN_EQUAL: 	return AST_BINARY_ASSIGN;
		case TOKEN_DOT: 	return AST_BINARY_ATTRIBUTE;
		case TOKEN_ADD: 	return AST_BINARY_ADD;
		case TOKEN_SUB: 	return AST_BINARY_SUB;
		case TOKEN_MUL: 	return AST_BINARY_MUL;
		case TOKEN_DIV: 	return AST_BINARY_DIV;
		default: 			return AST_BINARY_UNINITIALIZED;
	};
}

short Ast_Binary::getPrecedence (Token_Type opToken) {
	switch (opToken) {
		case TOKEN_EQUAL: return 1;
		case TOKEN_DOT:   return 1;
		case TOKEN_ADD:   return 2;
		case TOKEN_SUB:   return 2;
		case TOKEN_MUL:   return 3;
		case TOKEN_DIV:   return 3;
		default: 		  return 0;
	}
}

bool Ast_Binary::getLeftAssociativity (Token_Type opToken) {
	switch (opToken) {
		case TOKEN_EQUAL: return false;
		case TOKEN_DOT:   return false;
		case TOKEN_ADD:   return false;
		case TOKEN_SUB:   return false;
		case TOKEN_MUL:   return false;
		case TOKEN_DIV:   return false;
		default: 		  return false;
	}
}

void Ast_Unary::setOP (Token_Type tType) {
	this->unary_op = this->typeToOP(tType);
}

Ast_Unary_Type Ast_Unary::typeToOP (Token_Type tType) {
	switch (tType) {
		case TOKEN_SUB: return AST_UNARY_NEGATE_NUMBER;
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

Ast_Declaration* ast_make_declaration (Ast_Type_Definition* ty_def) {
    auto decl = new Ast_Declaration();
    decl->name = ty_def->name;
    decl->expression = ty_def;
    return decl;
}
