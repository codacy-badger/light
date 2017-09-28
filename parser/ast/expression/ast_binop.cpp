#pragma once

#include "ast_expression.cpp"

#include "lexer/lexer.cpp"

using namespace std;

class ASTBinop : public ASTExpression {
public:
	enum OP { ASSIGN, ATTR, ADD, SUB, MUL, DIV, COUNT };
	static map<Token::Type, bool> isLeftAssociate;
	static map<ASTBinop::OP, const char*> opChar;
	static map<Token::Type, short> precedence;

	OP op = OP::COUNT;
	ASTExpression* lhs = NULL;
	ASTExpression* rhs = NULL;

	ASTBinop (Token::Type tType) {
		this->setOP(tType);
	}

	void setOP (Token::Type tType) {
		this->op = this->typeToOP(tType);
	}

	OP typeToOP (Token::Type tType) {
		switch (tType) {
			case Token::Type::EQUAL: return OP::ASSIGN;
			case Token::Type::DOT: return OP::ATTR;
			case Token::Type::ADD: return OP::ADD;
			case Token::Type::SUB: return OP::SUB;
			case Token::Type::MUL: return OP::MUL;
			case Token::Type::DIV: return OP::DIV;
			default:
				cout << "[ERROR] Binary operator unknown: " << tType << "\n";
				exit(1);
		};
		return OP::COUNT;
	}

	void print (int tabs) {
		cout << "(";
		this->lhs->print(tabs);
		cout << " " << opChar[this->op] << " ";
		this->rhs->print(tabs);
		cout << ")";
	}

	static short getPrecedence (Token::Type opToken) {
		auto it = ASTBinop::precedence.find(opToken);
		if (it != ASTBinop::precedence.end())
			return ASTBinop::precedence[opToken];
		else return -1;
	}

	static bool getLeftAssociativity (Token::Type opToken) {
		auto it = ASTBinop::isLeftAssociate.find(opToken);
		if (it != ASTBinop::isLeftAssociate.end())
			return ASTBinop::isLeftAssociate[opToken];
		else return false;
	}
};

map<ASTBinop::OP, const char*> ASTBinop::opChar = {
	{ASTBinop::OP::ASSIGN, "="}, {ASTBinop::OP::ATTR, "."},
	{ASTBinop::OP::ADD, "+"}, {ASTBinop::OP::SUB, "-"},
	{ASTBinop::OP::MUL, "*"}, {ASTBinop::OP::DIV, "/"}
};
map<Token::Type, short> ASTBinop::precedence = {
	{Token::Type::EQUAL, 1}, {Token::Type::DOT, 1},
	{Token::Type::ADD, 2}, {Token::Type::SUB, 2},
	{Token::Type::MUL, 3}, {Token::Type::DIV, 3}
};
map<Token::Type, bool> ASTBinop::isLeftAssociate = {
	{Token::Type::EQUAL, false}, {Token::Type::DOT, false},
	{Token::Type::ADD, false}, {Token::Type::SUB, false},
	{Token::Type::MUL, false}, {Token::Type::DIV, false}
};
