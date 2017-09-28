#pragma once

#include <string>

#include "ast/type/ast_type.cpp"

class ParserContext {
public:
	map<std::string, ASTType*> types;

	ASTType* getType (std::string name) {
		auto it = this->types.find(name);
		if (it != this->types.end()) {
			return this->types[name];
		} else return nullptr;
	}

	void addType (ASTType* ty) {
		auto it = this->types.find(ty->name);
		if (it == this->types.end()) {
			this->types[ty->name] = ty;
		} else {
			cout << "ERROR: Type redeclaration: " << ty->name << "\n";
		}
	}
};
