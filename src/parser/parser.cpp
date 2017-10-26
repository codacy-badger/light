#pragma once

#include "parser/parser.hpp"

#include "compiler.hpp"

template <typename T>
T* setup_ast_node (Lexer* lexer, T* node) {
	node->filename = lexer->buffer->source;
	node->line = lexer->buffer->line;
	node->col = lexer->buffer->col;
	return node;
}
#define AST_NEW(T, ...) setup_ast_node(lexer, new T(__VA_ARGS__))

Parser::Parser (Light_Compiler* compiler, const char* filepath) {
	this->compiler = compiler;
	this->lexer = new Lexer(filepath);
}

Ast_Block* Parser::top_level_block () {
	auto _block = AST_NEW(Ast_Block, this->current_block);
	_block->list.push_back(ast_make_declaration(Light_Compiler::type_def_void));
	_block->list.push_back(ast_make_declaration(Light_Compiler::type_def_i1));
	_block->list.push_back(ast_make_declaration(Light_Compiler::type_def_i32));
	this->block(_block);
	return _block;
}

void Parser::block (Ast_Block* insert_block) {
	this->current_block = insert_block;
	this->on_block_begin(this->current_block);

	Ast_Statement* stm;
	while (stm = this->statement()) {
		if (stm->stm_type == AST_STATEMENT_IMPORT) {
			auto imp = static_cast<Ast_Import*>(stm);
			if (imp->import_flags & IMPORT_INCLUDE_CONTENT) {
				this->lexer->push(imp->filepath);
				if (this->lexer->buffer->is_valid()) {
					auto include_block = AST_NEW(Ast_Block, this->current_block);
					this->block(include_block);
					this->current_block->list.insert(this->current_block->list.end(),
						include_block->list.begin(), include_block->list.end());
					delete include_block;
				} else this->compiler->report_error(imp,
					"Can't open import file: '%s'", imp->filepath);
				this->lexer->pop();
			}
		} else {
			this->current_block->list.push_back(stm);
			if (this->current_block->parent == NULL) {
				this->to_next(stm);
			} else {
				if (stm->stm_type == AST_STATEMENT_DECLARATION) {
					auto decl = static_cast<Ast_Declaration*>(stm);
					if (decl->decl_flags & DECL_FLAG_CONSTANT)
						this->to_next(stm);
				}
			}
		}

		if (this->lexer->isNextType(TOKEN_EOF)) break;
	}

	this->on_block_end(this->current_block);
	this->current_block = this->current_block->parent;
}

Ast_Statement* Parser::statement () {
	switch (this->lexer->nextType) {
		case TOKEN_EOF: this->lexer->report_unexpected();
		case TOKEN_STM_END: {
			this->lexer->skip(1);
			return NULL;
		}
		case TOKEN_RETURN: {
			this->lexer->skip(1);
			auto output = AST_NEW(Ast_Return);
			output->exp = this->expression();
			this->lexer->check_skip(TOKEN_STM_END);
			return output;
		}
		case TOKEN_IMPORT: {
			this->lexer->skip(1);
			auto output = AST_NEW(Ast_Import);

			if (this->lexer->optional_skip(TOKEN_EXCLAMATION))
				output->import_flags |= IMPORT_INCLUDE_CONTENT;
			else if (this->lexer->optional_skip(TOKEN_AT))
				output->import_flags |= IMPORT_IS_NATIVE;

			if (this->lexer->isNextType(TOKEN_STRING))
				output->filepath = this->lexer->text();

			this->lexer->optional_skip(TOKEN_STM_END);
			return output;
		}
		case TOKEN_HASH: {
			this->lexer->skip(1);
			auto note = AST_NEW(Ast_Note);
			note->name = this->lexer->text();
			auto next_stm = this->statement();
			next_stm->notes.push_back(note);
			return next_stm;
		}
		case TOKEN_BRAC_OPEN: {
			this->lexer->skip(1);
			auto _block = AST_NEW(Ast_Block, this->current_block);
			this->block(_block);
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
			return _block;
		}
		case TOKEN_ID: {
			auto ident = this->ident();
			if (ident) {
				if (this->lexer->isNextType(TOKEN_COLON)) {
					auto decl = AST_NEW(Ast_Declaration);
					decl->name = ident->name;
					this->lexer->skip(1);
					decl->type = this->type_instance();

					if (this->lexer->isNextType(TOKEN_COLON)) {
						this->lexer->skip(1);
						decl->decl_flags |= DECL_FLAG_CONSTANT;
					} else if (this->lexer->isNextType(TOKEN_EQUAL)) {
						this->lexer->skip(1);
					} else if (this->lexer->isNextType(TOKEN_STM_END)) {
						this->lexer->skip(1);
						return decl;
					}

					decl->expression = this->expression();
					if (this->lexer->isNextType(TOKEN_STM_END))
						this->lexer->skip(1);
					return decl;
				} else {
					auto exp = this->expression(ident);
					if (exp) this->lexer->check_skip(TOKEN_STM_END);
					return exp;
				}
			}
		}
		default: {
			auto exp = this->expression();
			if (exp) this->lexer->check_skip(TOKEN_STM_END);
			return exp;
		}
	}
}

Ast_Declaration* Parser::declaration () {
	if (this->lexer->isNextType(TOKEN_ID)) {
		auto decl = AST_NEW(Ast_Declaration);
		decl->name = this->ident()->name;

		if (this->lexer->isNextType(TOKEN_COLON)) {
			this->lexer->skip(1);
			decl->type = this->type_instance();
		}

		if (this->lexer->isNextType(TOKEN_COLON)) {
			this->lexer->skip(1);
			decl->decl_flags |= DECL_FLAG_CONSTANT;
		} else if (this->lexer->isNextType(TOKEN_EQUAL)) {
			this->lexer->skip(1);
		}

		if (!this->lexer->isNextType(TOKEN_STM_END))
			decl->expression = this->expression();

		return decl;
	} else return NULL;
}

Ast_Type_Definition* Parser::type_definition () {
	return NULL;
}

Ast_Named_Type* Parser::struct_type (string name) {
	return NULL;
}

Ast_Type_Instance* Parser::type_instance () {
	if (this->lexer->isNextType(TOKEN_ID)) {
		auto ty_inst = AST_NEW(Ast_Named_Type);
		ty_inst->name = this->lexer->text();
		return ty_inst;
	} else if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
		auto ty_inst = AST_NEW(Ast_Function_Type);
		this->lexer->skip(1);

		Ast_Type_Instance* _tmp;
		while (_tmp = this->type_instance())
			ty_inst->parameters.push_back(_tmp);
		this->lexer->check_skip(TOKEN_PAR_CLOSE);

		if (this->lexer->isNextType(TOKEN_ARROW)) {
			this->lexer->skip(1);
			ty_inst->return_type = this->type_instance();
		} else {
			// FIXME: hack to make it work, the primitive types should be
			// inmediately found in the global scope
			ty_inst->return_type = AST_NEW(Ast_Named_Type, "void");
		}

		return ty_inst;
	} else return NULL;
}

Ast_Function* Parser::function () {
	return NULL;
}

Ast_Function_Type* Parser::_functionType () {
	return NULL;
}

void Parser::_functionParameters (vector<Ast_Declaration*>* output) {
	return;
}

Ast_Expression* Parser::expression (Ast_Ident* initial, short minPrecedence) {
	Ast_Expression* output;
    if (output = this->_atom(initial)) {
        Token_Type tt = this->lexer->nextType;
		auto precedence = Ast_Binary::getPrecedence(tt);
		while (precedence >= minPrecedence) {
			this->lexer->skip(1);

			int nextMinPrec = precedence;
			if (Ast_Binary::getLeftAssociativity(tt))
				nextMinPrec += 1;

			Ast_Binary* _tmp = AST_NEW(Ast_Binary, tt);
			_tmp->rhs = this->expression(NULL, nextMinPrec);
			_tmp->lhs = output;
			output = _tmp;

			tt = this->lexer->nextType;
			precedence = Ast_Binary::getPrecedence(tt);
		}
    }
	return output;
}

Ast_Expression* Parser::_atom (Ast_Ident* initial) {
	if (this->lexer->isNextType(TOKEN_ID) || initial) {
		auto output = initial ? initial : this->ident();
		if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
			return this->call(output);
		} else return output;
	} else if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
		this->lexer->skip(1);
		auto result = this->expression();
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
		return result;
	} else if (this->lexer->isNextType(TOKEN_FUNCTION)) {
		this->lexer->skip(1);

		auto fn = AST_NEW(Ast_Function);

		auto fn_type = AST_NEW(Ast_Function_Type);
		if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
			this->lexer->skip(1);
			Ast_Declaration* decl;
			while (decl = this->declaration()) {
				fn_type->parameters.push_back(decl->type);
				decl = this->declaration();
			}
			this->lexer->check_skip(TOKEN_PAR_CLOSE);
		}
		if (this->lexer->isNextType(TOKEN_ARROW)) {
			this->lexer->skip(1);
			fn_type->return_type = this->type_instance();
		} else {
			// FIXME: hack to make it work, the primitive types should be
			// inmediately found in the global scope
			fn_type->return_type = AST_NEW(Ast_Named_Type, "void");
		}
		fn->type = fn_type;

		if (!this->lexer->isNextType(TOKEN_STM_END)) {
			this->lexer->check_skip(TOKEN_BRAC_OPEN);
			fn->scope = AST_NEW(Ast_Block, this->current_block);
			this->block(fn->scope);
			this->lexer->check_skip(TOKEN_BRAC_CLOSE);
		}

		return fn;
	} else if (this->lexer->isNextType(TOKEN_SUB)) {
		this->lexer->skip(1);
		auto unop = AST_NEW(Ast_Unary, TOKEN_SUB);
		unop->exp = this->_atom();
		return unop;
	} else if (this->lexer->isNextType(TOKEN_ADD)) {
		this->lexer->skip(1);
		return this->expression();
	} else return this->literal();
}

Ast_Literal* Parser::literal () {
	Ast_Literal* output = NULL;
	switch (this->lexer->nextType) {
		case TOKEN_STRING: {
			output = AST_NEW(Ast_Literal);
			output->literal_type = AST_LITERAL_STRING;
			output->string_value = this->lexer->text();
			break;
		}
		case TOKEN_NUMBER: {
			output = AST_NEW(Ast_Literal);
			output->literal_type = AST_LITERAL_INTEGER;
			output->integer_value = atoi(this->lexer->text());
			break;
		}
		default: break;
	}
	return output;
}

Ast_Function_Call* Parser::call (Ast_Expression* callee) {
	Ast_Function_Call* output = NULL;
	if (this->lexer->isNextType(TOKEN_PAR_OPEN)) {
		this->lexer->skip(1);
		output = AST_NEW(Ast_Function_Call);
		output->fn = callee;
		Ast_Expression* exp = NULL;
		while (exp = this->expression()) {
			output->parameters.push_back(exp);
			if (this->lexer->isNextType(TOKEN_COMMA))
				this->lexer->skip(1);
		}
		this->lexer->check_skip(TOKEN_PAR_CLOSE);
	}
	return output;
}

Ast_Ident* Parser::ident () {
	if (this->lexer->isNextType(TOKEN_ID)) {
		Ast_Ident* output = AST_NEW(Ast_Ident);
		output->name = this->lexer->text();
		return output;
	} else return NULL;
}
