#pragma once


using namespace std;

class ASTStatementReturn : public ASTStatement {
	public:
		vector<ASTExpression*> expressions;

		void print (int tabs) {
			this->tabs(tabs);
			cout << "RETURNS ";
			if (this->expressions.size() > 0) {
				auto value = this->expressions[0];
				value->print(tabs);
				for(std::vector<int>::size_type i = 1; i < this->expressions.size(); i++) {
					cout << ", ";
					this->expressions[i]->print(tabs);
				}
			}
			cout << endl;
		}
};
