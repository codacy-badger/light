#pragma once

#include <string>
#include <vector>
#include <map>

#include "../pipes.cpp"
#include "parser/ast/ast.hpp"

struct FnDeps {
	ASTFunction* fn;
	std::vector<std::string> deps;
};

struct AddrDeps {
	std::map<std::string, std::vector<void**>> nameToAddrs;
};

struct NameResolutionPipe : Pipe {

	void onFunction (ASTFunction* fn) {
		this->check(fn, nullptr);
		this->toNext(fn);
	}

	void check (ASTFunction* fn, AddrDeps* deps) {
		cout << "Check Function: " << fn->name << "\n";
		this->check(fn->type, deps);
	}

	void onType (ASTType* ty) {
		this->check(ty, nullptr);
		this->toNext(ty);
	}

	void check (ASTType* ty, AddrDeps* deps) {
		if (auto unTy = dynamic_cast<ASTUnresolvedTy*>(ty)) {
			cout << "Unresolved Type: " << unTy->name << "\n";
		} else if (auto obj = dynamic_cast<ASTStructType*>(ty)) check(obj, deps);
		else if (auto obj = dynamic_cast<ASTPointerType*>(ty))  check(obj, deps);
		else if (auto obj = dynamic_cast<ASTFnType*>(ty))  		check(obj, deps);
	}

	void check (ASTStructType* ty, AddrDeps* deps) {
		cout << "Check Struct: " << ty->name << "\n";
	}

	void check (ASTPointerType* ty, AddrDeps* deps) {
		this->check(ty->base, deps);
	}

	void check (ASTFnType* ty, AddrDeps* deps) {
		this->check(ty->retType, deps);
	}
};
