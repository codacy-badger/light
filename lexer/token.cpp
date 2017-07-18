using namespace std;

#define ENUM_STR(name) case Type::name: return #name

class Token {
	public:
		enum Type {
			NONE,

			EQUAL,
			ADD,
			SUB,
			DIV,
			MUL,
			LET,
			ARROW,
			TYPE,
			FUNCTION,
			STM_END,
			RETURN,
			PAR_OPEN,
			PAR_CLOSE,
			BRAC_OPEN,
			BRAC_CLOSE,
			SQ_BRAC_OPEN,
			SQ_BRAC_CLOSE,
			DOT,
			COMMA,
			ID,
			NUMBER,
			STRING,
		};

		Type type;
		string text;
		unsigned int line, col;

		Token () {
			this->type = Type::NONE;
			this->text = "";
			this->line = 0;
			this->col = 0;
		}

		Token (Type type, unsigned int line, unsigned int col) {
			this->type = type;
			this->line = line;
			this->col = col;
		}

		Token (Type type, unsigned int line, unsigned int col, const char* text) {
			this->text = text;
			this->type = type;
			this->line = line;
			this->col = col;
		}

		Token (Token* token) {
			this->copy(token);
		}

		void copy (Token* token) {
			this->type = token->type;
			this->text = token->text;
			this->line = token->line;
			this->col = token->col;
		}

		static const char* typeToString (Type type) {
			switch (type) {
				ENUM_STR(NONE);

				ENUM_STR(EQUAL);
				ENUM_STR(ADD);
				ENUM_STR(SUB);
				ENUM_STR(DIV);
				ENUM_STR(MUL);
				ENUM_STR(LET);
				ENUM_STR(ARROW);
				ENUM_STR(TYPE);
				ENUM_STR(FUNCTION);
				ENUM_STR(STM_END);
				ENUM_STR(RETURN);
				ENUM_STR(PAR_OPEN);
				ENUM_STR(PAR_CLOSE);
				ENUM_STR(BRAC_OPEN);
				ENUM_STR(BRAC_CLOSE);
				ENUM_STR(SQ_BRAC_OPEN);
				ENUM_STR(SQ_BRAC_CLOSE);
				ENUM_STR(DOT);
				ENUM_STR(COMMA);
				ENUM_STR(ID);
				ENUM_STR(NUMBER);

				default: return "?";
			}
		}
};

ostream& operator<<(ostream& os, const Token& token) {
      os << "Token { [" << Token::typeToString(token.type) << "] ";
	  if (!token.text.empty()) os << "'" << token.text << "' ";
	  os << "@ " << token.line << "," << token.col << " }";
      return os;
}
