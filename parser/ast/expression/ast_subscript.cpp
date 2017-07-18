#pragma once


using namespace std;

class ASTSubscript : public ASTExpression {
	public:
		ASTExpression* expression = NULL;
		ASTExpression* index = NULL;

		void print (int tabs) {
			this->expression->print(tabs);
			cout << "[";
			this->index->print(tabs);
			cout << "]";
		}
};
