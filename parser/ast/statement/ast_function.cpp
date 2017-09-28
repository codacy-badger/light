#pragma once

#include <iostream>
#include <vector>

using namespace std;

class ASTFnType {
public:
	vector<ASTVarDef*> params;
	ASTType* retType = nullptr;

	void print (int tabs) {
		cout << "( ";
		if (this->params.size() > 0) {
			this->params[0]->printSimple(tabs);
			for (int i = 1; i < this->params.size(); i++) {
				cout << ", ";
				this->params[i]->printSimple(tabs);
			}
		}
		cout << " )";
		if (this->retType != nullptr) {
			cout << " -> ";
			this->retType->print(tabs);
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
