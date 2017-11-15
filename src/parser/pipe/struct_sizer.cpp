#include "parser/pipe/struct_sizer.hpp"

#include "compiler.hpp"

uint16_t get_type_size (Ast_Type_Definition* defn_ty) {
	switch (defn_ty->typedef_type) {
		case AST_TYPEDEF_STRUCT: {
			auto _struct = static_cast<Ast_Struct_Type*>(defn_ty);
			return _struct->byte_size;
		}
		default: return 0;
	}
}

void compute_struct_size (Ast_Struct_Type* _struct) {
	if (_struct->byte_size == 0) {
		uint16_t size = 0;
		for (int i = 0; i < _struct->attributes.size(); i++) {
			auto decl = _struct->attributes[i];
			decl->struct_byte_offset = size;
			decl->struct_index = i;

			assert (decl->type->exp_type == AST_EXPRESSION_TYPE_DEFINITION);
			auto defn_ty = static_cast<Ast_Type_Definition*>(decl->type);
			size += get_type_size(defn_ty);
		}
		_struct->byte_size = size;
	}
}

void Struct_Sizer::on_statement(Ast_Statement* stm) {
	if (stm->stm_type == AST_STATEMENT_DECLARATION) {
		auto decl = static_cast<Ast_Declaration*>(stm);
		if (decl->type == Light_Compiler::inst->type_def_type) {
			auto defn_ty = static_cast<Ast_Type_Definition*>(decl->expression);
			if (defn_ty->typedef_type == AST_TYPEDEF_STRUCT) {
				compute_struct_size(static_cast<Ast_Struct_Type*>(defn_ty));
			}
		}
	}
	this->to_next(stm);
}
