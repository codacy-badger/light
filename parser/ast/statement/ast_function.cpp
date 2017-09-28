#pragma once

#include <iostream>
#include <vector>

using namespace std;

class ASTFnType {
public:
	vector<ASTVarDef*> params;
	ASTType* retType = nullptr;

	void print (int tab) {
		cout << "(";
		for(auto const& type: this->params) {
			type->print(tab);
			cout << ", ";
		}
		cout << ")";
		if (this->retType != nullptr) {
			cout << " -> ";
			this->retType->print(tab);
		}
	}
};

class ASTFunction : public ASTStatement {
public:
	std::string name = "";
	ASTFnType* fnType = nullptr;
	ASTStatement* stms = nullptr;

	void print (int tabs) {
		cout << "fn " << this->name << " ";
		this->fnType->print(tabs);
		cout << "\n";
		if (this->stms != nullptr)
			this->stms->print(tabs + 1);
	}
};
