#pragma once

#include <string>
#include <vector>

struct ASTVariable;
struct ASTFunction;

struct ASTType : ASTExpression {
	std::vector<ASTVariable*> attrs;
	std::vector<ASTFunction*> methods;

	virtual bool isPrimitive () = 0;

	virtual bool isConstant() { return true; }
	virtual ASTType* getType() {
		//TODO: return special Type for consistency
		return nullptr;
	}
};

struct ASTAliasType : ASTType {
	std::string name;
	ASTType* type = nullptr;

	bool isPrimitive () {
		return type->isPrimitive();
	}
};

struct ASTFnType : ASTType {
	std::vector<ASTVariable*> params;
	ASTType* retType = nullptr;

	bool isPrimitive () { return false; }
};

//TODO: ASTCompositeType

struct ASTStructType : ASTType {
	std::string name;

	ASTStructType (std::string name = "") { this->name = name; }
	bool isPrimitive () { return false; }
};

//TODO: ASTArrayType (ASTCompositeType)

struct ASTPrimitiveType : ASTType {
	std::string name;

	ASTPrimitiveType (std::string name) { this->name = name; }
	bool isPrimitive () { return true; }

	static ASTType* _void;
	static ASTType* _i32;
};

ASTType* ASTPrimitiveType::_void = new ASTPrimitiveType("void");
ASTType* ASTPrimitiveType::_i32 = new ASTPrimitiveType("i32");
