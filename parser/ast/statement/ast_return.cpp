#pragma once

#include "parser/ast/type/ast_type.cpp"
#include "parser/ast/expression/ast_expression.cpp"

#include <iostream>

using namespace std;

class ASTReturn : public ASTStatement {
public:
	ASTExpression* exp = nullptr;
};
