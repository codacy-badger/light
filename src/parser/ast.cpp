#pragma once

#include "parser/ast.hpp"

Ast_Block::Ast_Block (string name, Ast_Block* parent) {
	this->type = AST_BLOCK;
	this->parent = parent;
	this->name = name;
}

void Ast_Block::add (string name, Ast_Expression* val) {
	auto it = this->symbols.find(name);
	if (it == this->symbols.end()) {
		this->symbols[name] = val;
	} else {
		cout << "ERROR: name collision: " << name << "\n";
		exit(1);
	}
}

Ast_Expression* Ast_Block::get (string name) {
	auto it = this->symbols.find(name);
	if (it != this->symbols.end()) {
		return this->symbols[name];
	} else {
		if (this->parent)
			return this->parent->get(name);
		else return nullptr;
	}
}

void AST_Binary::setOP (Token::Type tType) {
	this->op = this->typeToOP(tType);
}

Ast_Binary_Type AST_Binary::typeToOP (Token::Type tType) {
	switch (tType) {
		case Token::Type::EQUAL: 	return AST_BINARY_ASSIGN;
		case Token::Type::DOT: 		return AST_BINARY_ATTRIBUTE;
		case Token::Type::ADD: 		return AST_BINARY_ADD;
		case Token::Type::SUB: 		return AST_BINARY_SUB;
		case Token::Type::MUL: 		return AST_BINARY_MUL;
		case Token::Type::DIV: 		return AST_BINARY_DIV;
		default: {
			cout << "[ERROR] Binary operator unknown: " << tType << "\n";
			return AST_BINARY_UNINITIALIZED;
		}
	};
}

short AST_Binary::getPrecedence (Token::Type opToken) {
	auto it = AST_Binary::precedence.find(opToken);
	if (it != AST_Binary::precedence.end())
		return AST_Binary::precedence[opToken];
	else return -1;
}

bool AST_Binary::getLeftAssociativity (Token::Type opToken) {
	auto it = AST_Binary::isLeftAssociate.find(opToken);
	if (it != AST_Binary::isLeftAssociate.end())
		return AST_Binary::isLeftAssociate[opToken];
	else return false;
}

void AST_Unary::setOP (Token::Type tType) {
	this->op = this->typeToOP(tType);
}

Ast_Unary_Type AST_Unary::typeToOP (Token::Type tType) {
	switch (tType) {
		case Token::Type::SUB: return AST_UNARY_NEGATE;
		default: {
			cout << "[ERROR] Unary operator unknown: " << tType << "\n";
			return AST_UNARY_UNINITIALIZED;
		}
	};
}

map<Token::Type, short> AST_Binary::precedence = {
	{Token::Type::EQUAL, 	1}, {Token::Type::DOT, 1},
	{Token::Type::ADD, 		2}, {Token::Type::SUB, 2},
	{Token::Type::MUL, 		3}, {Token::Type::DIV, 3}
};
map<Token::Type, bool> AST_Binary::isLeftAssociate = {
	{Token::Type::EQUAL, 	false}, {Token::Type::DOT, false},
	{Token::Type::ADD, 		false}, {Token::Type::SUB, false},
	{Token::Type::MUL, 		false}, {Token::Type::DIV, false}
};

Ast_Type_Definition* Ast_Primitive_Type::_void = new Ast_Primitive_Type("void");
Ast_Type_Definition* Ast_Primitive_Type::_i1 =   new Ast_Primitive_Type("i1");
Ast_Type_Definition* Ast_Primitive_Type::_i8 =   new Ast_Primitive_Type("i8");
Ast_Type_Definition* Ast_Primitive_Type::_i16 =  new Ast_Primitive_Type("i16");
Ast_Type_Definition* Ast_Primitive_Type::_i32 =  new Ast_Primitive_Type("i32");
Ast_Type_Definition* Ast_Primitive_Type::_i64 =  new Ast_Primitive_Type("i64");
Ast_Type_Definition* Ast_Primitive_Type::_i128 = new Ast_Primitive_Type("i128");
