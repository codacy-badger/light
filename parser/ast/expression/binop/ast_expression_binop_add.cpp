#pragma once


using namespace std;

class ASTExpressionBinopAdd : public ASTExpressionBinop {
	public:
		void print (int tabs) {
			cout << "(";
			this->lhs->print(tabs);
			cout << " + ";
			this->rhs->print(tabs);
			cout << ")";
		}
};
