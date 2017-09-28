#pragma once

#include "ast_expression.cpp"

#include "lexer/lexer.cpp"

using namespace std;

class ASTUnop : public ASTExpression {
public:
	enum OP { NEG, COUNT };
	static map<ASTUnop::OP, const char*> opChar;

	OP op = OP::COUNT;
	ASTExpression* expression = nullptr;

	ASTUnop (Token::Type tType) {
		this->setOP(tType);
	}

	void setOP (Token::Type tType) {
		this->op = this->typeToOP(tType);
	}

	OP typeToOP (Token::Type tType) {
		switch (tType) {
			case Token::Type::SUB: return OP::NEG;
			default:
				cout << "PANIC -> " << tType << "\n";
				exit(87);
		};
		return OP::COUNT;
	}

	void print (int tabs) {
		cout << "(" << opChar[this->op] << " ";
		this->expression->print(tabs);
		cout << ")";
	}

	bool isConstant () {
		return this->expression->isConstant();
	}
};

map<ASTUnop::OP, const char*> ASTUnop::opChar = {
	{ASTUnop::OP::NEG, "-"}
};
