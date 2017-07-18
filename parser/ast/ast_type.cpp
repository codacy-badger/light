#pragma once


using namespace std;

#include "ast_node.cpp"

class ASTType : public ASTNode{
	public:
		string name;

		void print (int tabs) {
			if (this->name == "") cout  << "[?]";
			else cout << "[" << this->name << "]";
		}
};


