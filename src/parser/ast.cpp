#pragma once

#include "parser/ast.hpp"

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

Ast_Type_Definition* Ast_Primitive_Type::_void = new Ast_Primitive_Type("void");
Ast_Type_Definition* Ast_Primitive_Type::_i1 =   new Ast_Primitive_Type("i1");
Ast_Type_Definition* Ast_Primitive_Type::_i8 =   new Ast_Primitive_Type("i8");
Ast_Type_Definition* Ast_Primitive_Type::_i16 =  new Ast_Primitive_Type("i16");
Ast_Type_Definition* Ast_Primitive_Type::_i32 =  new Ast_Primitive_Type("i32");
Ast_Type_Definition* Ast_Primitive_Type::_i64 =  new Ast_Primitive_Type("i64");
Ast_Type_Definition* Ast_Primitive_Type::_i128 = new Ast_Primitive_Type("i128");
