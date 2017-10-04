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

struct ASTRef : ASTMemory {
	ASTMemory* memory = nullptr;

	bool isConstant() {
		return this->memory->isConstant();
	}

	ASTType* getType() {
		// TODO: return some sort of custom pointer type
		return nullptr;
	}
};

struct ASTDeref : ASTMemory {
	ASTMemory* memory = nullptr;

	bool isConstant() {
		return this->memory->isConstant();
	}

	ASTType* getType() {
		// TODO: return some sort of custom pointer type
		return nullptr;
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
