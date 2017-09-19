#pragma once

#include "ast_expression.cpp"

#include "lexer/lexer.cpp"

using namespace std;

class ASTUnop : public ASTExpression {
public:
	enum OPS { NEG, COUNT };
	static const char* OPSChar[];

	OPS op = OPS::COUNT;
	ASTExpression* expression = NULL;

	ASTUnop (Token::Type tType) {
		this->setOP(tType);
	}

	void setOP (Token::Type tType) {
		this->op = this->typeToOP(tType);
	}

	OPS typeToOP (Token::Type tType) {
		switch (tType) {
			case Token::Type::SUB: return OPS::NEG;
			default:
				cout << "PANIC -> " << tType << "\n";
				exit(87);
		};
		return OPS::COUNT;
	}

	void print (int tabs) {
		cout << "(" << OPSChar[this->op] << " ";
		this->expression->print(tabs);
		cout << ")";
	}
};

const char* ASTUnop::OPSChar[] = { "-" };
