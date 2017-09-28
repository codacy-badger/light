#pragma once

#include <iostream>
#include <vector>

using namespace std;

class ASTStatements : public ASTStatement {
	public:
		vector<ASTStatement*> list;
};
