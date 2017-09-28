#pragma once

#include "ast_expression.cpp"

#include "lexer/lexer.cpp"

using namespace std;

class ASTUnop : public ASTExpression {
public:
	enum OP { NEG, COUNT };
	static map<ASTUnop::OP, const char*> opChar;

	OP op = OP::COUNT;
	ASTExpression* exp = nullptr;

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

	bool isConstant () {
		return this->exp->isConstant();
	}

	ASTType* getType(ParserContext* context) {
		return this->exp->getType(context);
	}
};

map<ASTUnop::OP, const char*> ASTUnop::opChar = {
	{ASTUnop::OP::NEG, "-"}
};
