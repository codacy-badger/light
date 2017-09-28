#pragma once

#include <vector>
#include <string>

class ASTStatement : public ASTNode {
public:
	virtual ~ASTStatement() {}
};

struct ASTStatements : ASTStatement {
	std::vector<ASTStatement*> list;
};

struct ASTDefType : ASTStatement {
	std::string name = "";
	ASTStatements* stms = nullptr;
};

struct ASTVarDef : ASTStatement {
	ASTType* type;
	std::string name = "";
	ASTExpression* expression = nullptr;
};

struct ASTReturn : ASTStatement {
	ASTExpression* exp = nullptr;
};

//NOTE: this should provably be a subtype of ASTType
struct ASTFnType : ASTNode {
	std::vector<ASTVarDef*> params;
	ASTType* retType = nullptr;
};

struct ASTFunction : ASTStatement {
	std::string name;
	ASTFnType* fnType = nullptr;
	ASTStatement* stms = nullptr;
};

struct ASTExpStatement : ASTStatement {
	ASTExpression* exp = nullptr;
};
