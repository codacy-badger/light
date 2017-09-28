#pragma once

struct ASTVariable : ASTExpression {
	bool isConstant () {
		// TODO: oh the irony...
		return false;
	}

	ASTType* getType(ASTContext* context) = 0;
};

struct ASTId : ASTVariable {
	std::string name;

	ASTId (std::string name = "") {
		this->name = name;
	}

	ASTType* getType(ASTContext* context) {
		return context->getVarType(this->name);
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

struct ASTSubscript : ASTVariable {
	ASTExpression* expression = NULL;
	ASTExpression* index = NULL;

	ASTType* getType(ASTContext* context) {
		// TODO: store variables in context to query type
		return nullptr;
	}
};
