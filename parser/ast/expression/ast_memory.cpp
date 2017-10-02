#pragma once

struct ASTMemory : ASTValue {
	~ASTMemory () {}
};

struct ASTVariable : ASTMemory {
	std::string name = "";
	ASTType* type = nullptr;
	ASTExpression* expression = nullptr;

	bool isConstant() { return false; }
	ASTType* getType() {
		return this->type;
	}
};

struct ASTAttr : ASTMemory {
	ASTExpression* exp = nullptr;
	std::string name;

	ASTAttr (ASTExpression* exp = nullptr) {
		this->exp = exp;
	}

	bool isConstant() { return false; }
	ASTType* getType() {
		// TODO: store variables in context to query type
		return nullptr;
	}
};
