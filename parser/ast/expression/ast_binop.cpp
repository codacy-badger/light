#pragma once

#include "ast_expression.cpp"

#include "lexer/lexer.cpp"

using namespace std;

class ASTBinop : public ASTExpression {
public:
	enum OPS { ADD, SUB, MUL, DIV, COUNT };
	static const char* OPSChar[];

	OPS op = OPS::COUNT;
	ASTExpression* lhs = NULL;
	ASTExpression* rhs = NULL;

	ASTBinop (Token::Type tType) {
		this->setOP(tType);
	}

	void setOP (Token::Type tType) {
		this->op = this->typeToOP(tType);
	}

	OPS typeToOP (Token::Type tType) {
		switch (tType) {
			case Token::Type::ADD: return OPS::ADD;
			case Token::Type::SUB: return OPS::SUB;
			case Token::Type::MUL: return OPS::MUL;
			case Token::Type::DIV: return OPS::DIV;
			default:
				cout << "PANIC -> " << tType << "\n";
				exit(87);
		};
		return OPS::COUNT;
	}

	void print (int tabs) {
		cout << "(";
		this->lhs->print(tabs);
		cout << " " << OPSChar[this->op] << " ";
		this->rhs->print(tabs);
		cout << ")";
	}
};

const char* ASTBinop::OPSChar[] = { "+", "-", "*", "/" };
