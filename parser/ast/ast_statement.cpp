#pragma once

#include <vector>
#include <string>

struct ASTType;
struct ASTFunction;
struct ASTExpression;

struct ASTStatement : ASTNode {
	virtual ~ASTStatement() {}

	template <typename Callback>
	void forEach (Callback cb) {
		if (auto stms = dynamic_cast<ASTScope*>(this))
			stms->forEach(cb);
		else cb(this);
	}
};

struct ASTReturn : ASTStatement {
	ASTExpression* exp = nullptr;
};

struct ASTScope : ASTStatement {
	std::vector<ASTType*> types;
	std::vector<ASTFunction*> functions;
	std::vector<ASTStatement*> list;

	template <typename Callback>
	void forEach (Callback cb) {
		for (auto const& ty : types) {
			cb(ty);
			if (auto strTy = dynamic_cast<ASTStructType*>(ty)) {
				for (auto const& fn : strTy->methods) {
					cb(fn);
					if (fn->stm != nullptr)
						fn->stm->forEach(cb);
				}
			}
		}
		for (auto const& fn : functions) {
			cb(fn);
			if (fn->stm != nullptr)
				fn->stm->forEach(cb);
		}
		for (auto const& stm : list) stm->forEach(cb);
	}
};
