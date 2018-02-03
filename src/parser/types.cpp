#include "parser/types.hpp"

#include "compiler.hpp"

Ast_Pointer_Type* Types::get_pointer_type (Ast_Expression* base_type) {
	auto it = this->ptr_types.find(base_type);
    if (it != this->ptr_types.end()) {
		return it->second;
    } else {
		auto ptr_type = new Ast_Pointer_Type(base_type);
		this->ptr_types[base_type] = ptr_type;
		ast_compute_type_name_if_needed(ptr_type);
		return ptr_type;
	}
}

Ast_Slice_Type* Types::get_slice_type (Ast_Expression* base_type) {
	auto it = this->sli_types.find(base_type);
    if (it != this->sli_types.end()) {
		return it->second;
    } else {
		auto sli_type = new Ast_Slice_Type(base_type);
		this->sli_types[base_type] = sli_type;
		ast_compute_type_name_if_needed(sli_type);
		return sli_type;
	}
}

Ast_Function_Type* Types::get_function_type (Ast_Function* func) {
    auto func_type = new Ast_Function_Type();
    func_type->location = func->location;
    for (auto arg_decl : func->arg_decls) {
        func_type->arg_types.push_back(arg_decl->type);
    }
    func_type->ret_type = func->ret_type;
	ast_compute_type_name_if_needed(func_type);
    return func_type;
}

bool Types::is_implicid_cast (Ast_Type_Instance* type_from, Ast_Type_Instance* type_to) {
	if (type_from->is_signed && type_to->is_signed) {
		return type_to->byte_size >= type_from->byte_size;
	} else if (!type_from->is_signed && type_to->is_signed) {
		return type_to->byte_size > type_from->byte_size;
	} else if (type_from->is_signed && !type_to->is_signed) {
		return false;
	} else {
		// @Hack @Fixme this allows cast from u8 to *u8, which should not be allowed.
		return type_to->byte_size >= type_from->byte_size;
	}
}
