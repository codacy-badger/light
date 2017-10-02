#pragma once

#include <vector>
#include <string>

struct ASTType;
struct ASTFunction;
struct ASTExpression;

struct ASTStatement : ASTNode {
	virtual ~ASTStatement() {}
};

struct ASTReturn : ASTStatement {
	ASTExpression* exp = nullptr;
};

struct ASTScope : ASTStatement {
	std::vector<ASTType*> types;
	std::vector<ASTFunction*> functions;
	std::vector<ASTStatement*> list;

	template <typename ScopeCallback>
	void forEachScope (ScopeCallback cb) {
		cb(this);
		for (auto const& stm : this->list) {
			if (auto obj = dynamic_cast<ASTScope*>(stm)) cb(obj);
		}
		for (auto const& fn : this->functions) {
			if (auto obj = dynamic_cast<ASTScope*>(fn->stm)) cb(obj);
		}
	}

	template <typename VarCallback>
	void forEachVariable (VarCallback cb) {
		this->forEachScope([&](ASTScope* stms) {
			for (auto const& stm : stms->list) {
				if (auto var = dynamic_cast<ASTVariable*>(stm)) cb(var);
			}
		});
	}
};
