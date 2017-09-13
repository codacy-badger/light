#include <string>
#include <ostream>
#include <iostream>
#include <vector>

#include "parser/parser.cpp"

class Compiler {
	public:
		Compiler () { /* empty */ }

		void addSource (const char* filename) {
			this->sources.push_back(filename);
		}

		ASTStatements* compile () {
			ASTStatements* stms = new ASTStatements();
			for (auto const& filename: this->sources) {
				std::cout << "Compiling source: '" << filename << "'..." << std::endl;
				handleFile(stms, filename);
			}
			return stms;
		}

	private:
		vector<const char*> sources;

		void handleFile (ASTStatements* stms, const char* filename) {
			Parser* parser = new Parser(filename);
			ASTStatement* stm = parser->statement();
			while (stm != NULL) {
				stms->list.push_back(stm);
				stm = parser->statement();
			}
		}
};
