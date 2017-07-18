#pragma once


using namespace std;

class ASTNode {
	public:
		virtual void print (int tabs) = 0;

		void tabs (int tabs) {
			for (int _count = 0; _count < tabs; _count++) {
				printf("    ");
			}
		}

	private:
		static int _count;
};
