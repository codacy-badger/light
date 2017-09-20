#pragma once

#include <iostream>
#include <vector>

using namespace std;

class ASTFnParam {
public:
	string name = "";
	ASTType* type = nullptr;
	ASTExpression* defValue = nullptr;

	void print (int tab) {
		this->type->print(tab);
		cout << " " << name;
		if (defValue != nullptr) {
			cout << " -> ";
			this->defValue->print(tab);
		}
	}
};

class ASTFnType {
public:
	vector<ASTFnParam*> params;
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
