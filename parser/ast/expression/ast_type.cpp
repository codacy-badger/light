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

struct ASTPointerType : ASTType {
	ASTType* base = nullptr;

	bool isPrimitive () {
		return base->isPrimitive();
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
	static ASTType* _i1;
	static ASTType* _i8;
	static ASTType* _i16;
	static ASTType* _i32;
	static ASTType* _i64;
	static ASTType* _i128;
};

ASTType* ASTPrimitiveType::_void = new ASTPrimitiveType("void");
ASTType* ASTPrimitiveType::_i1 =   new ASTPrimitiveType("i1");
ASTType* ASTPrimitiveType::_i8 =   new ASTPrimitiveType("i8");
ASTType* ASTPrimitiveType::_i16 =  new ASTPrimitiveType("i16");
ASTType* ASTPrimitiveType::_i32 =  new ASTPrimitiveType("i32");
ASTType* ASTPrimitiveType::_i64 =  new ASTPrimitiveType("i64");
ASTType* ASTPrimitiveType::_i128 = new ASTPrimitiveType("i128");
