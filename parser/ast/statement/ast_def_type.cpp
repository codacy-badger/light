#pragma once

#include <iostream>
#include <vector>

#include "ast_var_def.cpp"

using namespace std;

class ASTDefType : public ASTStatement {
public:
	std::string name = "";
	ASTStatements* stms = nullptr;
};
