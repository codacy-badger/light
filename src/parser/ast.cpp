#pragma once

#include "parser/ast.hpp"

#include <string.h>

void Ast_Block::find_all_declarations (std::vector<Ast_Declaration*>* decls) {
    for (auto stm : this->list) {
        if (stm->stm_type == AST_STATEMENT_DECLARATION)
            decls->push_back(static_cast<Ast_Declaration*>(stm));
    }
}

void AST_Binary::setOP (Token_Type tType) {
	this->binary_op = this->typeToOP(tType);
}

Ast_Binary_Type AST_Binary::typeToOP (Token_Type tType) {
	switch (tType) {
		case TOKEN_EQUAL: 	return AST_BINARY_ASSIGN;
		case TOKEN_DOT: 		return AST_BINARY_ATTRIBUTE;
		case TOKEN_ADD: 		return AST_BINARY_ADD;
		case TOKEN_SUB: 		return AST_BINARY_SUB;
		case TOKEN_MUL: 		return AST_BINARY_MUL;
		case TOKEN_DIV: 		return AST_BINARY_DIV;
		default: {
			cout << "[ERROR] Binary operator unknown: " << tType << "\n";
			return AST_BINARY_UNINITIALIZED;
		}
	};
}

short AST_Binary::getPrecedence (Token_Type opToken) {
	auto it = AST_Binary::precedence.find(opToken);
	if (it != AST_Binary::precedence.end())
		return AST_Binary::precedence[opToken];
	else return -1;
}

bool AST_Binary::getLeftAssociativity (Token_Type opToken) {
	auto it = AST_Binary::isLeftAssociate.find(opToken);
	if (it != AST_Binary::isLeftAssociate.end())
		return AST_Binary::isLeftAssociate[opToken];
	else return false;
}

void AST_Unary::setOP (Token_Type tType) {
	this->unary_op = this->typeToOP(tType);
}

Ast_Unary_Type AST_Unary::typeToOP (Token_Type tType) {
	switch (tType) {
		case TOKEN_SUB: return AST_UNARY_NEGATE_NUMBER;
		default: {
			cout << "[ERROR] Unary operator unknown: " << tType << "\n";
			return AST_UNARY_UNINITIALIZED;
		}
	};
}

map<Token_Type, short> AST_Binary::precedence = {
	{TOKEN_EQUAL, 	1}, {TOKEN_DOT, 1},
	{TOKEN_ADD, 		2}, {TOKEN_SUB, 2},
	{TOKEN_MUL, 		3}, {TOKEN_DIV, 3}
};
map<Token_Type, bool> AST_Binary::isLeftAssociate = {
	{TOKEN_EQUAL, 	false}, {TOKEN_DOT, false},
	{TOKEN_ADD, 		false}, {TOKEN_SUB, false},
	{TOKEN_MUL, 		false}, {TOKEN_DIV, false}
};

bool Ast_Ident::operator ==(const Ast_Ident* other) const {
	return strcmp(this->name, other->name) == 0;
}

Ast_Ident* ast_make_ident (const char* name) {
	auto out = new Ast_Ident();
	out->name = name;
	return out;
}
