using namespace std;

#include <stdlib.h>

#include "../buffer/buffer.cpp"
#include "../buffer/file_buffer.cpp"

#include "../lexer/lexer.cpp"

#include "ast/ast_type.cpp"
#include "ast/ast_def_returns.cpp"
#include "ast/ast_def_parameters.cpp"
#include "ast/expression/ast_expression.cpp"
#include "ast/statement/ast_statements.cpp"

#define PARSER_DEBUG false

class Parser {
	public:
		Parser (const char* filename) {
			this->lexer = new Lexer(filename);
			this->source = filename;
		}

		Parser (PushbackBuffer* buffer) {
			this->lexer = new Lexer(buffer);
			this->source = "<buffer>";
		}

		ASTType* type () {
			if (this->lexer->isNextType(Token::Type::ID)) {
				ASTType* output = new ASTType();
				output->name = this->lexer->nextText();
				return output;
			} else if(this->lexer->isNextType(Token::Type::LET)) {
				this->lexer->skip(1);
				ASTType* output = new ASTType();
				output->name = "";
				return output;
			} else return NULL;
		}

		ASTDefParameter* def_parameter () {
			ASTType* type = this->type();
			if (type == NULL) return NULL;

			ASTDefParameter* param = new ASTDefParameter();
			param->type = type;

			if (this->lexer->isNextType(Token::Type::ID)) {
				param->name = this->lexer->nextText();
			} else error("Expected ID after parameter type");

			/*if (this->lexer->isNextType(Token::Type::EQUAL)) {
				this->lexer->skip(1);
				param->value = this->expression();
				if (param->value == NULL)
					error("Expected expression after equal in variable definition");
			}*/

			return param;
		}

		ASTDefParameters* def_parameters () {
			if (this->lexer->isNextType(Token::Type::PAR_OPEN))
				this->lexer->skip(1);
			else return NULL;

			ASTDefParameters* params = new ASTDefParameters();
			ASTDefParameter* param = this->def_parameter();
			while (param != NULL) {
				params->list.push_back(param);
				if (this->lexer->isNextType(Token::Type::COMMA)) {
					this->lexer->skip(1);
					param = this->def_parameter();
				} else break;
			}

			if (this->lexer->isNextType(Token::Type::PAR_CLOSE))
				this->lexer->skip(1);
			else error("Expected ')' after parameters definition");
			return params;
		}

		ASTDefReturns* def_returns () {
			if (this->lexer->isNextType(Token::Type::ARROW))
				this->lexer->skip(1);
			else return NULL;

			ASTType* type = this->type();
			if (type == NULL) return NULL;

			ASTDefReturns* returns = new ASTDefReturns();
			while (type != NULL) {
				returns->list.push_back(type);
				if (this->lexer->isNextType(Token::Type::COMMA)) {
					this->lexer->skip(1);
					type = this->type();
				} else break;
			}
			return returns;
		}

		ASTVariable* variable () {
			if (this->lexer->isNextType(Token::Type::ID)) {
				ASTVariable* output = new ASTVariable();
				output->name = this->lexer->nextText();
				return output;
			} else return NULL;
		}

		ASTString* string () {
			if (this->lexer->isNextType(Token::Type::STRING)) {
				ASTString* output = new ASTString();
				output->value = this->lexer->nextText();
				return output;
			} else return NULL;
		}

		ASTNumber* number () {
			if (this->lexer->isNextType(Token::Type::NUMBER)) {
				ASTNumber* output = new ASTNumber();
				output->value = this->lexer->nextText();
				return output;
			} else return NULL;
		}

		ASTDefFunction* def_function () {
			if (this->lexer->isNextType(Token::Type::FUNCTION)) {
				this->lexer->skip(1);

				ASTDefFunction* output = new ASTDefFunction();
				if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
					output->parameters = this->def_parameters();
				} else output->parameters = NULL;
				if (this->lexer->isNextType(Token::Type::ARROW)) {
					output->returns = this->def_returns();
				} else output->returns = NULL;
				output->body = this->statement();

				return output;
			} else return NULL;
		}

		ASTDefType* def_type () {
			if (this->lexer->isNextType(Token::Type::TYPE)) {
				this->lexer->skip(1);

				ASTDefType* output = new ASTDefType();
				if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
					output->parameters = this->def_parameters();
				} else output->parameters = NULL;
				output->body = this->statement();

				return output;
			} else return NULL;
		}

		ASTExpression* expression () {
		    ASTExpression* lhs = this->term();
		    if (lhs != NULL) {
		        Token::Type tt = this->lexer->peekType(0);
		        while (tt == Token::Type::ADD || tt == Token::Type::SUB) {
					this->lexer->skip(1);
		            ASTExpressionBinop* binop = NULL;
		            if (tt == Token::Type::ADD) {
						binop = new ASTExpressionBinopAdd();
		            } else binop = new ASTExpressionBinopSub();
					binop->lhs = lhs;
		            binop->rhs = this->term();
		            if (binop->rhs == NULL)
						error("Expected term on the right");
		            lhs = binop;
					tt = this->lexer->peekType(0);
		        }
		        return lhs;
		    } else return NULL;
		}

		ASTExpression* term () {
			ASTExpression* lhs = this->factor();
			if (lhs != NULL) {
				Token::Type tt = this->lexer->peekType(0);
				while (tt == Token::Type::MUL || tt == Token::Type::DIV) {
					this->lexer->skip(1);
					ASTExpressionBinop* binop = NULL;
		            if (tt == Token::Type::MUL) {
						binop = new ASTExpressionBinopMul();
		            } else binop = new ASTExpressionBinopDiv();
					binop->lhs = lhs;
		            binop->rhs = this->factor();
		            if (binop->rhs == NULL)
						error("Expected factor on the right");
		            lhs = binop;
					tt = this->lexer->peekType(0);
		        }
		        return lhs;
			} else return NULL;
		}

		ASTExpression* factor () {
			if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
				this->lexer->skip(1);
				ASTExpression* exp = this->expression();
				if(this->lexer->isNextType(Token::Type::PAR_OPEN))
					this->lexer->skip(1);
				else error("Expected closing parenthesys after expression");
				return exp;
			} else if (this->lexer->isNextType(Token::Type::SUB)) {
				this->lexer->skip(1);
				ASTExpressionUnopNeg* exp = new ASTExpressionUnopNeg();
				exp->expression = this->factor();
				return (ASTExpression*) exp;
			} else if (this->lexer->isNextType(Token::Type::ADD)) {
				this->lexer->skip(1);
				return this->expression();
			} else if (this->lexer->isNextType(Token::Type::NUMBER)) {
				return this->number();
			} else if (this->lexer->isNextType(Token::Type::ID)) {
				return this->variable();
			} else if (this->lexer->isNextType(Token::Type::STRING)) {
				return this->string();
			} else if (this->lexer->isNextType(Token::Type::FUNCTION)) {
				return this->def_function();
			} else if (this->lexer->isNextType(Token::Type::TYPE)) {
				return this->def_type();
			} else return NULL;
		}

		ASTStatement* statement () {
			ASTStatement* stmt = (ASTStatement*) this->statements();
			if (stmt != NULL) return stmt;
			stmt = (ASTStatement*) this->statement_def_variable();
			if (stmt != NULL) return stmt;
			stmt = (ASTStatement*) this->statement_return();
			if (stmt != NULL) return stmt;
			return NULL;
		}

		ASTStatements* statements () {
			if (this->lexer->isNextType(Token::Type::BRAC_OPEN))
				this->lexer->skip(1);
			else return NULL;

			ASTStatements* output = new ASTStatements();
			ASTStatement* exp = this->statement();
			while (exp != NULL) {
				output->list.push_back(exp);
				exp = this->statement();
			}

			if (this->lexer->isNextType(Token::Type::BRAC_CLOSE))
				this->lexer->skip(1);
			else error("Expected '}' after statements");

			return output;
		}

		ASTStatementDefVariable* statement_def_variable () {
			ASTType* type = this->type();
			if (type == NULL) return NULL;

			ASTStatementDefVariable* output = new ASTStatementDefVariable();
			output->type = type;

			if (this->lexer->isNextType(Token::Type::ID))
				output->name = this->lexer->nextText();
			else error("Expected ID after type in variable declaration");

			if (this->lexer->isNextType(Token::Type::EQUAL)) {
				this->lexer->skip(1);
				output->expression = this->expression();
				if (output->expression == NULL)
					error("Expected expression after equal in variable definition");
			}

			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else error("Expected ';' after variable declaration");

			return output;
		}

		ASTStatementReturn* statement_return () {
			if (this->lexer->isNextType(Token::Type::RETURN))
				this->lexer->skip(1);
			else return NULL;

			ASTStatementReturn* output = new ASTStatementReturn();
			ASTExpression* exp = this->expression();
			if (exp == NULL) error("Expected expression after return statement");
			while (exp != NULL) {
				output->expressions.push_back(exp);
				if (this->lexer->isNextType(Token::Type::COMMA)) {
					this->lexer->skip(1);
					exp = this->expression();
				} else break;
			}

			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else error("Expected ';' after return statement");

			return output;
		}

	private:
		Token* token = new Token();
		const char* source = NULL;
		Lexer* lexer = NULL;

		void error (const char* message) {
			this->lexer->peek(this->token, 0);
			cout << "[Light] ERROR: " << message << endl;
			cout << "        in '" << this->source << "' @ " <<
				this->token->line << ", " << this->token->col << endl;
			exit(EXIT_FAILURE);
		}
};
