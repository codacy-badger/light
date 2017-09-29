#pragma once

#include <string>
#include <vector>

struct ASTVarDef;

struct ASTType : ASTStatement {
	std::string name;
	ASTStatements* stms = nullptr;

	ASTType (std::string name = "") { this->name = name; }

	bool isPrimitive () { return false; }
};

struct ASTPrimitiveType : ASTType {
	ASTPrimitiveType (std::string name) : ASTType(name) {}
	bool isPrimitive () { return true; }

	static ASTType* _void;
	static ASTType* _i32;
	static ASTType* _int;
};

ASTType* ASTPrimitiveType::_void = new ASTPrimitiveType("void");
ASTType* ASTPrimitiveType::_i32 = new ASTPrimitiveType("i32");
ASTType* ASTPrimitiveType::_int = new ASTPrimitiveType("int");

struct ASTFnType : ASTType {
	std::vector<ASTVarDef*> params;
	ASTType* retType = nullptr;
};
