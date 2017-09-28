#pragma once

#include "parser/ast/expression/ast_call.cpp"

#include <iostream>

using namespace std;

class ASTExpStatement : public ASTStatement {
public:
	ASTExpression* exp = nullptr;
};
