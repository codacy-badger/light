#pragma once

struct ASTVariable : ASTExpression {
	virtual ~ASTVariable() {}

	bool isConstant () {
		return false;
	}

	ASTType* getType(ASTContext* context) = 0;
};

struct ASTPointer : ASTVariable {
	std::string name;

	ASTPointer (std::string name = "") {
		this->name = name;
	}

	ASTType* getType(ASTContext* context) {
		return context->getVariable(this->name)->type;
	}
};

struct ASTAttr : ASTVariable {
	std::string name;
	ASTExpression* exp = nullptr;

	ASTAttr (ASTExpression* exp = nullptr) {
		this->exp = exp;
	}

	ASTType* getType(ASTContext* context) {
		// TODO: store variables in context to query type
		return nullptr;
	}
};
