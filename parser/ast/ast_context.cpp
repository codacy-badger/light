#pragma once

#include <string>

struct ASTExpression;

struct ASTContext {
	ASTContext* parent = nullptr;
	map<std::string, ASTExpression*> variables;

	ASTContext (ASTContext* parent = nullptr) {
		this->parent = parent;
		this->add("void", ASTPrimitiveType::_void);
		this->add("i32", ASTPrimitiveType::_i32);
		this->add("int", ASTPrimitiveType::_i32);
	}

	void add (std::string name, ASTExpression* val) {
		auto it = this->variables.find(name);
		if (it == this->variables.end()) {
			this->variables[name] = val;
		} else {
			cout << "ERROR: name collision: " << name << "\n";
		}
	}

	ASTExpression* get (std::string name) {
		auto it = this->variables.find(name);
		if (it != this->variables.end()) {
			return this->variables[name];
		} else return nullptr;
	}
};
