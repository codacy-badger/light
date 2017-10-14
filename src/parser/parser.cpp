#pragma once

#include "parser/parser.hpp"

bool Parser::block () {
	Ast_Statement* exp;
	while (exp = this->statement())
		this->currentScope->list.push_back(exp);
	return true;
}

Ast_Statement* Parser::statement () {
	switch (this->lexer->nextType) {
		case TOKEN_TYPE_STM_END: {
			this->lexer->skip(1);
			return nullptr;
		}
		case TOKEN_TYPE_TYPE: {
			return this->type();
		}
		case TOKEN_TYPE_FUNCTION: {
			return this->function();
		}
		case TOKEN_TYPE_LET: {
			return this->var_def();
		}
		case TOKEN_TYPE_RETURN: {
			return this->returnStm();
		}
		case TOKEN_TYPE_BRAC_OPEN: {
			this->lexer->skip(1);
			this->scopePush("<anon>");
			this->block();
			CHECK_TYPE(BRAC_CLOSE, "block");
			auto output = this->currentScope;
			this->scopePop();
			return output;
		}
		default: {
			auto exp = this->expression();
			if (exp) CHECK_TYPE(STM_END, "expression");
			return exp;
		}
	}
}

Ast_Type_Definition* Parser::type () {
	if (this->lexer->isNextType(TOKEN_TYPE_TYPE)) {
		this->lexer->skip(1);

		auto name = this->lexer->text();
		if (this->lexer->isNextType(TOKEN_TYPE_EQUAL)) {
			this->lexer->skip(1);
			auto result = this->_typeInstance();
			this->currentScope->add(name, result);
			CHECK_TYPE(STM_END, "type alias");
			return result;
		} else if (this->lexer->isNextType(TOKEN_TYPE_BRAC_OPEN)) {
			auto result = this->structType(name);
			this->toNext(result);
			return result;
		}
	}
	return nullptr;
}

Ast_Struct_Type* Parser::structType (string name) {
	auto output = AST_NEW(Ast_Struct_Type, name);
	this->currentScope->add(name, output);

	Ast_Statement* stm = nullptr;
	CHECK_TYPE(BRAC_OPEN, "type name");
	while (stm = this->_typeBody()) {
		if (auto var = dynamic_cast<Ast_Variable*>(stm)) {
			output->attrs.push_back(var);
			CHECK_TYPE(STM_END, "type attribute");
		} else if (auto fn = dynamic_cast<Ast_Function*>(stm))
			output->methods.push_back(fn);
		else expected("attribute or method", "type");
	}
	CHECK_TYPE(BRAC_CLOSE, "type body");

	return output;
}

Ast_Statement* Parser::_typeBody () {
	if (this->lexer->isNextType(TOKEN_TYPE_FUNCTION)) {
		return this->function();
	} else if (this->lexer->isNextType(TOKEN_TYPE_LET)) {
		this->lexer->skip(1);
		return this->_var_def();
	} else return this->_var_def();
}

Ast_Function* Parser::function () {
	if (this->lexer->isNextType(TOKEN_TYPE_FUNCTION)) {
		this->lexer->skip(1);
		auto output = AST_NEW(Ast_Function);
		if (this->lexer->isNextType(TOKEN_TYPE_ID))
			output->name = this->lexer->text();
		else expected("Identifier", "'fn' keyword");
		output->type = this->_functionType();
		this->currentScope->add(output->name, output);

		if (this->lexer->isNextType(TOKEN_TYPE_STM_END))
			this->lexer->skip(1);
		else if (this->lexer->isNextType(TOKEN_TYPE_BRAC_OPEN)) {
			this->scopePush(output->name);
			this->lexer->skip(1);
			for (auto const &param : output->type->params)
				this->currentScope->add(param->name, param);
			this->block();
			CHECK_TYPE(BRAC_CLOSE, "function body");
			output->stm = this->currentScope;
			this->scopePop();
		}

		this->toNext(output);
		return output;
	} else return nullptr;
}

Ast_Function_Type* Parser::_functionType () {
	auto output = AST_NEW(Ast_Function_Type);
	this->_functionParameters(&output->params);
	if (this->lexer->isNextType(TOKEN_TYPE_ARROW)) {
		this->lexer->skip(1);
		output->retType = this->_typeInstance();
	} else output->retType = Ast_Primitive_Type::_void;
	return output;
}

void Parser::_functionParameters (vector<Ast_Variable*>* output) {
	if (this->lexer->isNextType(TOKEN_TYPE_PAR_OPEN)) {
		this->lexer->skip(1);

		Ast_Variable* fnParam = nullptr;
		while (fnParam = this->_var_def()) {
			output->push_back(fnParam);
			if (this->lexer->isNextType(TOKEN_TYPE_COMMA))
				this->lexer->skip(1);
		}

		if(this->lexer->isNextType(TOKEN_TYPE_PAR_CLOSE))
			this->lexer->skip(1);
		else expected("closing parenthesys", "function parameters");
	}
}

Ast_Variable* Parser::var_def () {
	if (this->lexer->isNextType(TOKEN_TYPE_LET)) {
		this->lexer->skip(1);

		auto output = this->_var_def();
		CHECK_TYPE(STM_END, "variable declaration");

		this->currentScope->add(output->name, output);
		return output;
	} else return nullptr;
}

Ast_Variable* Parser::_var_def () {
	if (this->lexer->isNextType(TOKEN_TYPE_ID)) {
		auto output = AST_NEW(Ast_Variable);
		output->name = this->lexer->text();

		if (this->lexer->isNextType(TOKEN_TYPE_COLON)) {
			this->lexer->skip(1);
			output->type = this->_typeInstance();
		}

		if (this->lexer->isNextType(TOKEN_TYPE_EQUAL)) {
			this->lexer->skip(1);
			output->expression = this->expression();
			if (output->expression == nullptr)
				expected("expression", "'='");
		}
		return output;
	} else return nullptr;
}

Ast_Type_Definition* Parser::_typeInstance () {
	if (this->lexer->isNextType(TOKEN_TYPE_MUL)) {
		this->lexer->skip(1);
		auto ptrTy = AST_NEW(Ast_Pointer_Type);
		ptrTy->base = this->_typeInstance();
		return ptrTy;
	} else if (this->lexer->isNextType(TOKEN_TYPE_ID)) {
		auto typeName = this->lexer->text();
		auto output = this->currentScope->get<Ast_Type_Definition>(typeName);
		if (!output) output = AST_NEW(Ast_Unresolved_Type, typeName);
		return output;
	} else return nullptr;
}

Ast_Return* Parser::returnStm () {
	if (this->lexer->isNextType(TOKEN_TYPE_RETURN)) {
		this->lexer->skip(1);
		auto output = AST_NEW(Ast_Return);
		output->exp = this->expression();
		CHECK_TYPE(STM_END, "return expression");
		return output;
	} else return nullptr;
}

Ast_Expression* Parser::expression (short minPrecedence) {
	Ast_Expression* output;
    if (output = this->_atom()) {
        Token_Type tt = this->lexer->nextType;
		auto precedence = AST_Binary::getPrecedence(tt);
		while (precedence >= minPrecedence) {
			this->lexer->skip(1);

			int nextMinPrec = precedence;
			if (AST_Binary::getLeftAssociativity(tt))
				nextMinPrec += 1;

			AST_Binary* _tmp = AST_NEW(AST_Binary, tt);
			_tmp->rhs = this->expression(nextMinPrec);
			_tmp->lhs = output;
			output = _tmp;

			tt = this->lexer->nextType;
			precedence = AST_Binary::getPrecedence(tt);
		}
    }
	return output;
}

Ast_Expression* Parser::_atom () {
	if (this->lexer->isNextType(TOKEN_TYPE_PAR_OPEN)) {
		this->lexer->skip(1);
		auto result = this->expression();
		CHECK_TYPE(PAR_CLOSE, "expression");
		return result;
	} else if (this->lexer->isNextType(TOKEN_TYPE_SUB)) {
		this->lexer->skip(1);
		auto unop = AST_NEW(AST_Unary, TOKEN_TYPE_SUB);
		unop->exp = this->_atom();
		return unop;
	} else if (this->lexer->isNextType(TOKEN_TYPE_AMP)) {
		this->lexer->skip(1);
		auto deref = AST_NEW(Ast_Deref);
		auto exp = this->_atom();
		if (auto mem = dynamic_cast<AST_Memory*>(exp)) {
			deref->memory = mem;
		} // TODO: we should report a parsing error in else block
		return deref;
	} else if (this->lexer->isNextType(TOKEN_TYPE_MUL)) {
		this->lexer->skip(1);
		auto ref = AST_NEW(AST_Ref);
		auto exp = this->_atom();
		if (auto mem = dynamic_cast<AST_Memory*>(exp)) {
			ref->memory = mem;
		} // TODO: we should report a parsing error in else block
		return ref;
	} else if (this->lexer->isNextType(TOKEN_TYPE_ADD)) {
		this->lexer->skip(1);
		return this->expression();
	} else if (this->lexer->isNextType(TOKEN_TYPE_ID)) {
		auto output = this->variable();
		if (this->lexer->isNextType(TOKEN_TYPE_PAR_OPEN)) {
			auto fnPtr = reinterpret_cast<Ast_Function*>(output);
			return this->call(fnPtr);
		} else return output;
	} else return this->literal();
}

Ast_Literal* Parser::literal () {
	Ast_Literal* output = nullptr;
	if (this->lexer->isNextType(TOKEN_TYPE_STRING)) {
		output = AST_NEW(Ast_Literal, Ast_Literal::TYPE::STRING);
		output->stringValue = this->lexer->text();
	} else if (this->lexer->isNextType(TOKEN_TYPE_NUMBER)) {
		output = AST_NEW(Ast_Literal, Ast_Literal::TYPE::INT);
		output->intValue = atoi(this->lexer->text());
	}
	return output;
}

Ast_Function_Call* Parser::call (Ast_Expression* callee) {
	Ast_Function_Call* output = nullptr;
	if (this->lexer->isNextType(TOKEN_TYPE_PAR_OPEN)) {
		this->lexer->skip(1);
		output = AST_NEW(Ast_Function_Call);
		output->fn = callee;
		Ast_Expression* exp = nullptr;
		while (exp = this->expression()) {
			output->params.push_back(exp);
			if (this->lexer->isNextType(TOKEN_TYPE_COMMA))
				this->lexer->skip(1);
		}
		CHECK_TYPE(PAR_CLOSE, "function arguments");
	}
	return output;
}

Ast_Expression* Parser::variable () {
	if (this->lexer->isNextType(TOKEN_TYPE_ID)) {
		string name(this->lexer->nextText);
		auto output = this->currentScope->get(name);
		if (!output) output = AST_NEW(Ast_Unresolved_Expression, name);
		this->lexer->skip(1);

		Token_Type tt = this->lexer->nextType;
		while (tt == TOKEN_TYPE_DOT) {
			this->lexer->skip(1);
			if (tt == TOKEN_TYPE_DOT) {
				auto attr = AST_NEW(Ast_Attribute, output);
				if (this->lexer->isNextType(TOKEN_TYPE_ID))
					attr->name = this->lexer->text();
				else expected("name", "attribute access");
				output = attr;
				tt = this->lexer->nextType;
			}
		}

		return output;
	} else return nullptr;
}

void Parser::scopePush (string name) {
	assert(this->currentScope);
	this->currentScope = AST_NEW(Ast_Block, name, this->currentScope);
}

void Parser::scopePop () {
	assert(this->currentScope->parent);
	this->currentScope = this->currentScope->parent;
}

void Parser::expected (const char* expect, const char* after) {
	cout << "[Light] ERROR: Expected " << expect
		<< " after " << after << endl;
	cout << "        at ";
	this->lexer->buffer->printLocation();
	cout << endl;
	exit(EXIT_FAILURE);
}
