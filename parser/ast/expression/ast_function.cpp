#pragma once

#include <iostream>
#include <vector>

#include "parser/ast/statement/ast_statement.cpp"

using namespace std;

class ASTFunction : public ASTExpression {
public:
	std::string name = "";
	vector<ASTVarDef*> params;
	ASTType* retType = nullptr;
	ASTStatements* stms = nullptr;

	void print (int tabs) {
		cout << this->name << " ";
		this->printParamTypes(tabs);
		if (this->retType != nullptr) {
			cout << " -> ";
			this->retType->print(tabs);
		}
		if (this->stms != nullptr) {
			cout << "\n";
			this->stms->print(tabs + 1);
		}
	}

private:
	void printParamTypes (int tabs) {
		cout << "(";
		for(auto const& type: this->params) {
			type->print(tabs);
			cout << ", ";
		}
		cout << ")";
	}
};
