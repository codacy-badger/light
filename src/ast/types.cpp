#include "ast/types.hpp"

#include "compiler.hpp"

Ast_Struct_Type* Types::type_def_type = new Ast_Struct_Type("type",   0);
Ast_Struct_Type* Types::type_def_void = new Ast_Struct_Type("void",   0);
Ast_Struct_Type* Types::type_def_bool = new Ast_Struct_Type("bool",   1, true /* primitive */ );
Ast_Struct_Type* Types::type_def_s8   = new Ast_Struct_Type("s8", 	   1, true, true /* signed */ );
Ast_Struct_Type* Types::type_def_s16  = new Ast_Struct_Type("s16",    2, true, true);
Ast_Struct_Type* Types::type_def_s32  = new Ast_Struct_Type("s32",    4, true, true);
Ast_Struct_Type* Types::type_def_s64  = new Ast_Struct_Type("s64",    8, true, true);
Ast_Struct_Type* Types::type_def_u8   = new Ast_Struct_Type("u8",     1, true);
Ast_Struct_Type* Types::type_def_u16  = new Ast_Struct_Type("u16",    2, true);
Ast_Struct_Type* Types::type_def_u32  = new Ast_Struct_Type("u32",    4, true);
Ast_Struct_Type* Types::type_def_u64  = new Ast_Struct_Type("u64",    8, true);
Ast_Struct_Type* Types::type_def_f32  = new Ast_Struct_Type("f32",    4, true);
Ast_Struct_Type* Types::type_def_f64  = new Ast_Struct_Type("f64",    8, true);

Ast_Expression* Types::value_false = ast_make_literal(false);
Ast_Expression* Types::value_true = ast_make_literal(true);

void Types::add_type_if_new (Ast_Type_Instance* type) {
	if (type->guid < 0) {
		switch (type->typedef_type) {
			case AST_TYPEDEF_STRUCT: {
				auto _struct = static_cast<Ast_Struct_Type*>(type);
				this->add_struct_type_if_new(_struct);
				break;
			}
			case AST_TYPEDEF_POINTER: {
				auto ptr_type = static_cast<Ast_Pointer_Type*>(type);
				this->add_pointer_type_if_new(ptr_type);
				break;
			}
			case AST_TYPEDEF_ARRAY: {
				auto array_type = static_cast<Ast_Array_Type*>(type);
				this->add_array_type_if_new(array_type);
				break;
			}
			case AST_TYPEDEF_FUNCTION: {
				auto func_type = static_cast<Ast_Function_Type*>(type);
				this->add_function_type_if_new(func_type);
				break;
			}
			default: break;
		}
	}
}

void Types::add_struct_type_if_new (Ast_Struct_Type* _struct) {
	if (_struct->is_slice) {
		auto slice = static_cast<Ast_Slice_Type*>(_struct);
		auto base_type = static_cast<Ast_Type_Instance*>(slice->base);
		auto it = this->sli_types.find(base_type);
	    if (it == this->sli_types.end()) {
			ast_compute_type_name_if_needed(slice);
			this->sli_types[base_type] = slice;
			this->add_new_global_unique_type(slice);
		}
	} else {
		for (auto type_inst : this->all_types) {
			if (ast_types_are_equal(_struct, type_inst))
				return;
		}
		this->add_new_global_unique_type(_struct);
	}
}

void Types::add_pointer_type_if_new (Ast_Pointer_Type* ptr_type) {
	auto base_type = static_cast<Ast_Type_Instance*>(ptr_type->base);
	auto it = this->ptr_types.find(base_type);
	if (it == this->ptr_types.end()) {
		ast_compute_type_name_if_needed(ptr_type);
		this->ptr_types[base_type] = ptr_type;
		this->add_new_global_unique_type(ptr_type);
	}
}

void Types::add_array_type_if_new (Ast_Array_Type* array_type) {
	auto length = array_type->get_length();
	auto it = this->arr_types.find(array_type->base);
	if (it == this->arr_types.end()) {
		ast_compute_type_name_if_needed(array_type);
		this->arr_types[array_type->base][length] = array_type;
		this->add_new_global_unique_type(array_type);
	} else {
		auto sub_map = &this->arr_types[array_type->base];
		auto it2 = sub_map->find(length);
		if (it2 == sub_map->end()) {
			ast_compute_type_name_if_needed(array_type);
			(*sub_map)[length] = array_type;
			this->add_new_global_unique_type(array_type);
		} else {
			array_type->name = (*sub_map)[length]->name;
			array_type->guid = (*sub_map)[length]->guid;
		}
	}
}

void Types::add_function_type_if_new (Ast_Function_Type* func_type) {
	for (auto func_type2 : this->func_types) {
		if (ast_function_types_are_equal(func_type, func_type2)) {
			func_type->name = func_type2->name;
			func_type->guid = func_type2->guid;
			return;
		}
	}
	ast_compute_type_name_if_needed(func_type);
	this->func_types.push_back(func_type);
	this->add_new_global_unique_type(func_type);
}

void Types::add_new_global_unique_type (Ast_Type_Instance* type_inst) {
	type_inst->guid = this->all_types.size();
	this->all_types.push_back(type_inst);
}

Ast_Pointer_Type* Types::get_pointer_type (Ast_Expression* base) {
	if (base->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
		auto base_type = static_cast<Ast_Type_Instance*>(base);

		auto it = this->ptr_types.find(base_type);
	    if (it != this->ptr_types.end()) return it->second;
	    else {
			auto ptr_type = new Ast_Pointer_Type(base_type);
			ast_compute_type_name_if_needed(ptr_type);
			this->add_new_global_unique_type(ptr_type);
			this->ptr_types[base_type] = ptr_type;
			return ptr_type;
		}
	} else return new Ast_Pointer_Type(base);
}

Ast_Slice_Type* Types::get_slice_type (Ast_Expression* base) {
	if (base->exp_type == AST_EXPRESSION_TYPE_INSTANCE) {
		auto base_type = static_cast<Ast_Type_Instance*>(base);

		auto it = this->sli_types.find(base_type);
	    if (it != this->sli_types.end()) return it->second;
	    else {
			auto sli_type = new Ast_Slice_Type(base_type);
			ast_compute_type_name_if_needed(sli_type);
			this->add_new_global_unique_type(sli_type);
			this->sli_types[base_type] = sli_type;
			return sli_type;
		}
	} else return new Ast_Slice_Type(base);
}

bool Types::is_implicid_cast (Ast_Type_Instance* type_from, Ast_Type_Instance* type_to) {
	if (type_from->is_primitive && type_to->is_primitive) {
		if (type_to == Compiler::instance->types->type_def_bool) return true;
		else if (type_from->is_signed == type_to->is_signed) {
			return type_to->byte_size >= type_from->byte_size;
		} else if (!type_from->is_signed && type_to->is_signed) {
			return type_to->byte_size > type_from->byte_size;
		} else return false;
	} else return false;
}
