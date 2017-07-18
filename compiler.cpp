using namespace std;

#include <vector>

#include "parser/parser.cpp"

class Compiler {
	public:
		Compiler () { /* empty */ }

		void addSource (const char* filename) {
			this->sources.push_back(filename);
		}

		void compile (const char* output) {
			for (auto const& filename: this->sources) {
				cout << endl << "Compiling source: '" << filename << "'..." << endl;
				handleFile(filename);
			}
		}

	private:
		vector<const char*> sources;

		void handleFile (const char* filename) {
			Parser* parser = new Parser(filename);
			ASTStatement* stm = parser->statement();
			while (stm != NULL) {
				stm->print(0);
				stm = parser->statement();
			}
		}
};
