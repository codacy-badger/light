#pragma once

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "buffer/buffer.cpp"
#include "buffer/file_buffer.cpp"

#include "lexer/lexer.cpp"

#include "ast/ast.hpp"

using namespace std;

class Parser {
public:
	ASTContext* context = new ASTContext();

	Parser (const char* filename) {
		this->initParser(new Lexer(filename), filename);
	}

	Parser (PushbackBuffer* buffer) {
		this->initParser(new Lexer(buffer), "<buffer>");
	}

	bool typeInstance (ASTType** output) {
		if (this->lexer->isNextType(Token::Type::ID)) {
			auto typeName = this->lexer->text();
			auto typeExp = this->context->get(typeName);
			if (typeExp == nullptr) {
				this->context->addUnresolved(typeName, output);
			} else {
				(*output) = dynamic_cast<ASTType*>(typeExp);
				if ((*output) == nullptr) error("Name is not a type!");
			}
			return true;
		} else return false;
	}

	bool constant (ASTConst** output) {
		if (this->lexer->isNextType(Token::Type::STRING)) {
			(*output) = new ASTConst(ASTConst::TYPE::STRING);
			(*output)->stringValue = this->lexer->text();
			return true;
		} else if (this->lexer->isNextType(Token::Type::NUMBER)) {
			(*output) = new ASTConst(ASTConst::TYPE::INT);
			(*output)->intValue = atoi(this->lexer->text());
			return true;
		} else return nullptr;
	}

	bool call (ASTCall** output) {
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);
			(*output) = new ASTCall();
			ASTExpression* exp;
			while (this->expression(&exp)) {
				(*output)->params.push_back(exp);
				if (this->lexer->isNextType(Token::Type::COMMA))
					this->lexer->skip(1);
			}
			if(this->lexer->isNextType(Token::Type::PAR_CLOSE))
				this->lexer->skip(1);
			else expected("closing parenthesys", "call arguments");

			return true;
		} else return false;
	}

	bool variable (ASTExpression** output) {
		if (this->lexer->isNextType(Token::Type::ID)) {
			string name(this->lexer->nextText);
			(*output) = this->context->get(name);
			if ((*output) != nullptr) this->lexer->skip(1);

			Token::Type tt = this->lexer->nextType;
			while (tt == Token::Type::DOT) {
				this->lexer->skip(1);
				if (tt == Token::Type::DOT) {
					auto attr = new ASTAttr((*output));
					if (this->lexer->isNextType(Token::Type::ID))
						attr->name = this->lexer->text();
					else expected("name", "attribute access");
					(*output) = attr;
					tt = this->lexer->nextType;
				}
			}

			return output;
		} else return nullptr;
	}

	bool expression (ASTExpression** output, short minPrecedence = 1) {
	    if (this->atom(output)) {
	        Token::Type tt = this->lexer->nextType;
			auto precedence = ASTBinop::getPrecedence(tt);
			while (precedence >= minPrecedence) {
				this->lexer->skip(1);

				int nextMinPrec = precedence;
				if (ASTBinop::getLeftAssociativity(tt))
					nextMinPrec += 1;

				ASTBinop* _tmp = new ASTBinop(tt);
				this->expression(&_tmp->rhs, nextMinPrec);
				_tmp->lhs = (*output);
				(*output) = _tmp;

				tt = this->lexer->nextType;
				precedence = ASTBinop::getPrecedence(tt);
			}
	        return true;
	    } else return false;
	}

	bool statement (ASTStatement** output) {
		if (this->lexer->isNextType(Token::Type::STM_END)) {
			this->lexer->skip(1);
			return false;
		}

		if (this->type(reinterpret_cast<ASTStructType**>(output))) return true;
		if (this->function(reinterpret_cast<ASTFunction**>(output))) return true;
		if (this->var_def(reinterpret_cast<ASTVariable**>(output))) return true;
		if (this->returnStm(reinterpret_cast<ASTReturn**>(output))) return true;
		if (this->statements(reinterpret_cast<ASTStatements**>(output))) return true;
		if (this->expression(reinterpret_cast<ASTExpression**>(output))) {
			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else expected("';'", "expression");
			return true;
		} else return false;
	}

	bool statements (ASTStatements** output, bool forceBraquets = true) {
		if (forceBraquets) {
			if (this->lexer->isNextType(Token::Type::BRAC_OPEN))
				this->lexer->skip(1);
			else return false;
		}

		auto stms = new ASTStatements();
		ASTStatement* exp;
		while (this->statement(&exp)) {
			if (dynamic_cast<ASTType*>(exp)) {
				stms->list.insert(stms->list.begin(), exp);
			} else stms->list.push_back(exp);
			
		}
		(*output) = stms;

		if (forceBraquets) {
			if (this->lexer->isNextType(Token::Type::BRAC_CLOSE))
				this->lexer->skip(1);
			else expected("'}'", "statements");
		}

		return true;
	}

	bool type (ASTStructType** output) {
		if (this->lexer->isNextType(Token::Type::TYPE)) {
			this->lexer->skip(1);
			(*output) = new ASTStructType(this->lexer->text());

			ASTStatements* stms;
			if (this->statements(&stms)) {
				for (auto const &it : stms->list) {
					if (auto var = dynamic_cast<ASTVariable*>(it))
						(*output)->attrs.push_back(var);
					else if (auto fn = dynamic_cast<ASTFunction*>(it))
						(*output)->methods.push_back(fn);
					else error("Expected attribute or method inside type");
				}
			} else this->lexer->skip(1);
			this->context->add((*output)->name, (*output));
			return true;
		} else return false;
	}

	bool var_def (ASTVariable** output) {
		if (this->lexer->isNextType(Token::Type::LET)) {
			this->lexer->skip(1);

			this->_var_def(output);
			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else expected("';'", "variable declaration");

			this->context->add((*output)->name, (*output));
			return true;
		} else return false;
	}

	bool returnStm (ASTReturn** output) {
		if (this->lexer->isNextType(Token::Type::RETURN)) {
			this->lexer->skip(1);
			(*output) = new ASTReturn();
			this->expression(&(*output)->exp);
			if (this->lexer->isNextType(Token::Type::STM_END))
				this->lexer->skip(1);
			else expected("';'", "return expression");
			return true;
		} else return false;
	}

	bool function (ASTFunction** output) {
		if (this->lexer->isNextType(Token::Type::FUNCTION)) {
			this->lexer->skip(1);
			(*output) = new ASTFunction();
			if (this->lexer->isNextType(Token::Type::ID))
				(*output)->name = this->lexer->text();
			else expected("Identifier", "'fn' keyword");
			this->functionType(&(*output)->type);

			this->context = this->context->push();
			for (auto const &param : (*output)->type->params)
				this->context->add(param->name, param);
			this->statement(&(*output)->stms);
			this->context = this->context->pop();

			this->context->add((*output)->name, (*output));
			return true;
		} else return false;
	}

	bool program (ASTStatements** output) {
		bool result = this->statements(output, false);
		if (this->context->unresolved.size() > 0)
			error("Could not resolve all names!");
		return result;
	}

private:
	const char* source = nullptr;
	Lexer* lexer = nullptr;

	void initParser (Lexer* lexer, const char* source) {
		this->source = source;
		this->lexer = lexer;
	}

	bool atom (ASTExpression** output) {
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);
			this->expression(output);
			if(this->lexer->isNextType(Token::Type::PAR_CLOSE))
				this->lexer->skip(1);
			else error("Expected closing parenthesys after expression");
			return true;
		} else if (this->lexer->isNextType(Token::Type::SUB)) {
			this->lexer->skip(1);
			auto unop = new ASTUnop(Token::Type::SUB);
			this->atom(&unop->exp);
			(*output) = unop;
			return true;
		} else if (this->lexer->isNextType(Token::Type::ADD)) {
			this->lexer->skip(1);
			this->expression(output);
			return true;
		} else if (this->lexer->isNextType(Token::Type::ID)) {
			this->variable(output);
			string name;
			if ((*output) == nullptr)
				name = string(this->lexer->text());
			if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
				ASTCall* fnCall;
				this->call(&fnCall);
				if ((*output) == nullptr)
					this->context->addUnresolved(name, &fnCall->var);
				else fnCall->var = dynamic_cast<ASTFunction*>(*output);
				(*output) = fnCall;
				return true;
			} else return true;
		} else return this->constant(reinterpret_cast<ASTConst**>(output));
	}

	bool _var_def (ASTVariable** output) {
		if (this->lexer->isNextType(Token::Type::ID)) {
			(*output) = new ASTVariable();
			(*output)->name = this->lexer->text();

			if (this->lexer->isNextType(Token::Type::COLON)) {
				this->lexer->skip(1);
				if (this->lexer->isNextType(Token::Type::ID)) {
					this->typeInstance(&(*output)->type);
				}
			}

			if (this->lexer->isNextType(Token::Type::EQUAL)) {
				this->lexer->skip(1);
				this->expression(&(*output)->expression);
				if ((*output)->expression == nullptr)
					expected("expression", "'='");
			}

			if ((*output)->type == nullptr) {
				if ((*output)->expression != nullptr) {
					ASTType* ty = (*output)->expression->getType(context);
					if (ty != nullptr) {
						(*output)->type = ty;
					} else error("Type could not be inferred!");
				}
			}
			return output;
		} else return nullptr;
	}

	bool functionType (ASTFnType** output) {
		(*output) = new ASTFnType();
		this->functionParameters(&(*output)->params);
		if (this->lexer->isNextType(Token::Type::ARROW)) {
			this->lexer->skip(1);
			this->typeInstance(&(*output)->retType);
		} else (*output)->retType = ASTPrimitiveType::_void;
		return true;
	}

	bool functionParameters (vector<ASTVariable*>* output) {
		if (this->lexer->isNextType(Token::Type::PAR_OPEN)) {
			this->lexer->skip(1);

			ASTVariable* fnParam;
			while (this->_var_def(&fnParam)) {
				output->push_back(fnParam);
				if (this->lexer->isNextType(Token::Type::COMMA))
					this->lexer->skip(1);
			}

			if(this->lexer->isNextType(Token::Type::PAR_CLOSE))
				this->lexer->skip(1);
			else expected("closing parenthesys", "function parameters");
			return true;
		} else return false;
	}

	void error (const char* message) {
		cout << "[Light] ERROR: " << message << endl;
		cout << "        in '" << this->source << "' @ " <<
			this->lexer->buffer->line << ", " <<
			this->lexer->buffer->col << endl;
		exit(EXIT_FAILURE);
	}

	void expected (const char* expect, const char* after) {
		cout << "[Light] ERROR: Expected " << expect
			<< " after " << after << endl;
		cout << "        in '" << this->source << "' @ " <<
			this->lexer->buffer->line << ", " <<
			this->lexer->buffer->col << endl;
		exit(EXIT_FAILURE);
	}
};
