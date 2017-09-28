#pragma once

#include <iostream>
#include <vector>

using namespace std;

class ASTFnType : public ASTNode {
public:
	vector<ASTVarDef*> params;
	ASTType* retType = nullptr;
};

class ASTFunction : public ASTStatement {
public:
	std::string name = "";
	ASTFnType* fnType = nullptr;
	ASTStatement* stms = nullptr;
};
