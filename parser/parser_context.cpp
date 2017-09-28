#pragma once

#include <string>

#include "ast/type/ast_type.cpp"

class ParserContext {
public:
	ParserContext* parent = nullptr;
	map<std::string, ASTType*> types;

	ParserContext (ParserContext* parent = nullptr) {
		this->parent = parent;
		this->addType("i32", new ASTI32Type());
		this->addType("int", "i32");
	}

	ASTType* getType (std::string name) {
		auto it = this->types.find(name);
		if (it != this->types.end()) {
			return this->types[name];
		} else return nullptr;
	}

	void addType (std::string name, ASTType* ty) {
		auto it = this->types.find(name);
		if (it == this->types.end()) {
			this->types[name] = ty;
		} else {
			cout << "ERROR: Type redeclaration: " << name << "\n";
		}
	}

	void addType (ASTType* ty) {
		if (ty != nullptr) this->addType(ty->name, ty);
	}

	void addType (std::string alias, std::string original) {
		auto ty = this->getType(original);
		if (ty != nullptr) this->addType(alias, ty);
	}
};
