#pragma once


using namespace std;

class ASTStatementReturn : public ASTStatement {
	public:
		vector<ASTExpression*> expressions;

		void print (int tabs) {
			this->tabs(tabs);
			cout << "RETURNS ";
			for(auto const& value: this->expressions) {
				value->print(tabs);
				cout << ", ";
			}
			cout << endl;
		}
};
