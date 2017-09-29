#pragma once

#include <string>

struct ASTType;
struct ASTVarDef;

struct ASTContext {
	ASTContext* parent = nullptr;
	map<std::string, ASTType*> types;
	map<std::string, ASTVarDef*> variables;

	ASTContext (ASTContext* parent = nullptr) {
		this->parent = parent;
		this->addType(ASTPrimitiveType::_void);
		this->addType(ASTPrimitiveType::_i32);
		this->addType(ASTPrimitiveType::_int, ASTPrimitiveType::_i32);
	}

	void addVariable (std::string name, ASTVarDef* ty) {
		auto it = this->variables.find(name);
		if (it == this->variables.end()) {
			this->variables[name] = ty;
		} else {
			cout << "ERROR: Variable redeclaration: " << name << "\n";
		}
	}

	ASTVarDef* getVariable (std::string name) {
		auto it = this->variables.find(name);
		if (it != this->variables.end()) {
			return this->variables[name];
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

	void addType (ASTType* alias, ASTType* original) {
		if (original != nullptr) this->addType(alias->name, original);
	}

	ASTType* getType (std::string name) {
		auto it = this->types.find(name);
		if (it != this->types.end()) {
			return this->types[name];
		} else return nullptr;
	}
};
