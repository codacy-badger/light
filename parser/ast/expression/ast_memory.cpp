#pragma once

struct ASTMemory : ASTExpression {
	~ASTMemory () {}
};

struct ASTVariable : ASTMemory {
	std::string name = "";
	ASTType* type = nullptr;
	ASTExpression* expression = nullptr;

	bool isConstant() { return false; }
	ASTType* getType(ASTContext* context) {
		return this->type;
	}

	void onTypeResolved () {
		
	}
};

struct ASTAttr : ASTMemory {
	ASTExpression* exp = nullptr;
	std::string name;

	ASTAttr (ASTExpression* exp = nullptr) {
		this->exp = exp;
	}

	bool isConstant() { return false; }
	ASTType* getType(ASTContext* context) {
		// TODO: store variables in context to query type
		return nullptr;
	}
};
