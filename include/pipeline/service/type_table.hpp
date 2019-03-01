#pragma once

#include "ast/nodes.hpp"

#include "utils/array.hpp"

#define MAX_TYPE_NAME_LENGTH 1024

struct Type_Table {
    Build_Context* context = NULL;

    Array<Ast_Struct_Type*> internal_types = Array<Ast_Struct_Type*>(32);
    Array<Ast_Type*> global_type_table = Array<Ast_Type*>(32);
    uint8_t function_pointer_size = 0;
    uint8_t pointer_size = 0;

    char type_name_buffer[MAX_TYPE_NAME_LENGTH];
    
    Ast_Struct_Type* type_namespace = NULL;
	Ast_Struct_Type* type_type = NULL;
	Ast_Struct_Type* type_void = NULL;
	Ast_Struct_Type* type_bool = NULL;
	Ast_Struct_Type* type_byte = NULL;
	Ast_Struct_Type* type_s8 = NULL;
	Ast_Struct_Type* type_s16 = NULL;
	Ast_Struct_Type* type_s32 = NULL;
	Ast_Struct_Type* type_s64 = NULL;
	Ast_Struct_Type* type_u8 = NULL;
	Ast_Struct_Type* type_u16 = NULL;
	Ast_Struct_Type* type_u32 = NULL;
	Ast_Struct_Type* type_u64 = NULL;
	Ast_Struct_Type* type_f32 = NULL;
	Ast_Struct_Type* type_f64 = NULL;
	Ast_Struct_Type* type_string = NULL;
    Ast_Struct_Type* type_any = NULL;
    
    void init (Build_Context* c) {
        this->context = c;

        this->function_pointer_size = c->target_arch->register_size;
        this->pointer_size = c->target_arch->register_size;

        this->type_namespace    = new Ast_Struct_Type("namespace",  0);
        this->type_type         = new Ast_Struct_Type("type",       0, true /* primitive */ );
        this->type_void         = new Ast_Struct_Type("void",       0, true);
        this->type_bool         = new Ast_Struct_Type("bool",       1, true);
        this->type_byte         = new Ast_Struct_Type("byte",       1, true);
        this->type_s8           = new Ast_Struct_Type("s8",         1, true, true /* number */ , true /* signed */ );
        this->type_s16          = new Ast_Struct_Type("s16",        2, true, true, true);
        this->type_s32          = new Ast_Struct_Type("s32",        4, true, true, true);
        this->type_s64          = new Ast_Struct_Type("s64",        8, true, true, true);
        this->type_u8           = new Ast_Struct_Type("u8",         1, true, true);
        this->type_u16          = new Ast_Struct_Type("u16",        2, true, true);
        this->type_u32          = new Ast_Struct_Type("u32",        4, true, true);
        this->type_u64          = new Ast_Struct_Type("u64",        8, true, true);
        this->type_f32          = new Ast_Struct_Type("f32",        4, true, true);
        this->type_f64          = new Ast_Struct_Type("f64",        8, true, true);
        
        auto str_type = new Ast_Struct_Type("string");
        str_type->inferred_type = this->type_type;

        auto length_decl = new Ast_Declaration("length", this->type_u64);

        auto data_decl_type = new Ast_Pointer_Type(this->type_byte);
        auto data_decl = new Ast_Declaration("data", data_decl_type);

        str_type->scope.add(length_decl);
        str_type->scope.add(data_decl);

        this->type_string = str_type;

        auto any_type = new Ast_Struct_Type("any");
        any_type->inferred_type = this->type_type;

        auto type_decl = new Ast_Declaration("type", this->type_u8);

        auto value_decl_type = new Ast_Pointer_Type(this->type_void);
        auto value_decl = new Ast_Declaration("value", value_decl_type);

        any_type->scope.add(type_decl);
        any_type->scope.add(value_decl);

        this->type_any = any_type;
    }

    void unique (Ast_Type** type_ptr) {
        auto type = (*type_ptr);

        if (type->type_guid > 0) return;

        switch (type->typedef_type) {
            case AST_TYPEDEF_STRUCT: {
                this->unique(reinterpret_cast<Ast_Struct_Type**>(type_ptr));
                assert(type->type_guid > 0);
                break;
            }
            case AST_TYPEDEF_POINTER: {
                this->unique(reinterpret_cast<Ast_Pointer_Type**>(type_ptr));
                break;
            }
            case AST_TYPEDEF_ARRAY: {
                this->unique(reinterpret_cast<Ast_Array_Type**>(type_ptr));
                break;
            }
            case AST_TYPEDEF_FUNCTION: {
                this->unique(reinterpret_cast<Ast_Function_Type**>(type_ptr));
                break;
            }
            case AST_TYPEDEF_SLICE: {
                this->unique(reinterpret_cast<Ast_Slice_Type**>(type_ptr));
                break;
            }
            case AST_TYPEDEF_TUPLE: {
                this->unique(reinterpret_cast<Ast_Tuple_Type**>(type_ptr));
                break;
            }
            default: assert(false);
        }
    }

    void unique (Ast_Struct_Type** struct_ptr) {
        auto _struct = (*struct_ptr);

        if (_struct->type_guid > 0) return;

        this->add_unique(_struct);
    }

    void unique (Ast_Pointer_Type** ptr_type_ptr) {
        auto ptr_type = (*ptr_type_ptr);

        if (ptr_type->type_guid > 0) return;

        assert(ptr_type->base->exp_type == AST_EXPRESSION_TYPE);
        this->unique(&ptr_type->typed_base);

        For2 (this->global_type_table, type) {
            if (type->typedef_type == AST_TYPEDEF_POINTER) {
                auto uniqued_ptr_type = static_cast<Ast_Pointer_Type*>(type);
                assert(uniqued_ptr_type->base->exp_type == AST_EXPRESSION_TYPE);

                if (uniqued_ptr_type->typed_base->type_guid == ptr_type->typed_base->type_guid) {
                    (*ptr_type_ptr) = uniqued_ptr_type;
                    return;
                }
            }
        }

        ptr_type->byte_size = this->pointer_size;
        this->add_unique(ptr_type);
    }

    void unique (Ast_Array_Type** array_type_ptr) {
        auto array_type = (*array_type_ptr);

        if (array_type->type_guid > 0) return;

        assert(array_type->base->exp_type == AST_EXPRESSION_TYPE);
        this->unique(&array_type->typed_base);

        For2 (this->global_type_table, type) {
            if (type->typedef_type == AST_TYPEDEF_ARRAY) {
                auto uniqued_array_type = static_cast<Ast_Array_Type*>(type);
                assert(uniqued_array_type->base->exp_type == AST_EXPRESSION_TYPE);

                if (uniqued_array_type->typed_base->type_guid == array_type->typed_base->type_guid) {
                    (*array_type_ptr) = uniqued_array_type;
                    return;
                }
            }
        }

        this->add_unique(array_type);
    }

    void unique (Ast_Slice_Type** slice_type_ptr) {
        auto slice_type = (*slice_type_ptr);

        if (slice_type->type_guid > 0) return;

        assert(slice_type->base->exp_type == AST_EXPRESSION_TYPE);
        this->unique(&slice_type->typed_base);

        For2 (this->global_type_table, type) {
            if (type->typedef_type == AST_TYPEDEF_SLICE) {
                auto uniqued_slice_type = static_cast<Ast_Slice_Type*>(type);
                assert(uniqued_slice_type->base->exp_type == AST_EXPRESSION_TYPE);

                if (uniqued_slice_type->typed_base->type_guid == slice_type->typed_base->type_guid) {
                    (*slice_type_ptr) = uniqued_slice_type;
                    return;
                }
            }
        }

        this->add_slice_attributes(slice_type);
        this->add_unique(slice_type);
    }

    void unique (Ast_Function_Type** func_type_ptr) {
        auto func_type = (*func_type_ptr);

        if (func_type->type_guid > 0) return;

        For_Ref (func_type->arg_types) {
            assert((*it)->exp_type == AST_EXPRESSION_TYPE);
            this->unique((Ast_Type**) it);
        }
        assert(func_type->ret_type->exp_type == AST_EXPRESSION_TYPE);
        this->unique((Ast_Type**) &func_type->ret_type);

        For2 (this->global_type_table, type) {
            if (type->typedef_type == AST_TYPEDEF_FUNCTION) {
                auto uniqued_function_type = static_cast<Ast_Function_Type*>(type);

                if (this->types_are_equal(uniqued_function_type, func_type)) {
                    (*func_type_ptr) = uniqued_function_type;
                    return;
                }
            }
        }

        func_type->byte_size = this->function_pointer_size;
        this->add_unique(func_type);
    }

    void unique (Ast_Tuple_Type** tuple_type_ptr) {
        auto tuple_type = (*tuple_type_ptr);

        if (tuple_type->type_guid > 0) return;

        for (size_t i = 0; i < tuple_type->types.size; i++) {
            auto sub_type = tuple_type->types[i];

            if (!sub_type) return;

            assert(sub_type->exp_type == AST_EXPRESSION_TYPE);
            this->unique((Ast_Type**) &(tuple_type->types[i]));
        }

        For2 (this->global_type_table, type) {
            if (type->typedef_type == AST_TYPEDEF_TUPLE) {
                auto uniqued_tuple_type = static_cast<Ast_Tuple_Type*>(type);

                if (this->types_are_equal(uniqued_tuple_type, tuple_type)) {
                    (*tuple_type_ptr) = uniqued_tuple_type;
                    return;
                }
            }
        }

        this->add_unique(tuple_type);
    }

    void add_unique (Ast_Type* type) {
        type->type_guid = this->global_type_table.size + 1;
        this->global_type_table.push(type);
        this->compute_type_name_if_needed(type);
    }

    bool types_are_equal (Ast_Function_Type* func_type1, Ast_Function_Type* func_type2) {
        if (func_type1 == func_type2) return true;

        if (func_type1->arg_types.size != func_type2->arg_types.size) return false;
        if (func_type1->ret_type != func_type2->ret_type) return false;

        for (size_t i = 0; i < func_type1->arg_types.size; i++) {
            auto attr_type1 = func_type1->arg_types[i];
            auto attr_type2 = func_type2->arg_types[i];

            if (attr_type1 != attr_type2) return false;
        }

        return true;
    }

    bool types_are_equal (Ast_Tuple_Type* tuple_type1, Ast_Tuple_Type* tuple_type2) {
        if (tuple_type1 == tuple_type2) return true;

        if (tuple_type1->types.size != tuple_type2->types.size) return false;

        for (size_t i = 0; i < tuple_type1->types.size; i++) {
            auto type1 = tuple_type1->types[i];
            auto type2 = tuple_type2->types[i];

            if (type1 != type2) return false;
        }

        return true;
    }

    Ast_Pointer_Type* get_or_add_pointer_type (Ast_Type* base_type) {
        this->unique(&base_type);

        For2 (this->global_type_table, type) {
            if (type->typedef_type == AST_TYPEDEF_POINTER) {
                auto uniqued_ptr_type = static_cast<Ast_Pointer_Type*>(type);
                assert(uniqued_ptr_type->base->exp_type == AST_EXPRESSION_TYPE);

                if (uniqued_ptr_type->typed_base->type_guid == base_type->type_guid) {
                    return uniqued_ptr_type;
                }
            }
        }

        auto ptr_type = new Ast_Pointer_Type(base_type);
        ptr_type->type_guid = this->global_type_table.size + 1;
        ptr_type->byte_size = this->pointer_size;

        this->global_type_table.push(ptr_type);
        this->compute_type_name_if_needed(ptr_type);
        return ptr_type;
    }

    Ast_Tuple_Type* get_or_add_tuple_type (Ast_Comma_Separated* comma_separated) {
        auto tuple_type = new Ast_Tuple_Type();
        tuple_type->types.resize(comma_separated->expressions.size);
        For (comma_separated->expressions) {
            auto value = comma_separated->expressions[i];

            if (value) {
                assert(value->inferred_type);
                tuple_type->types[i] = value->inferred_type;
            } else tuple_type->types[i] = NULL;
        }
        this->unique(&tuple_type);
        return tuple_type;
    }

    Ast_Function_Type* get_or_add_function_type (Ast_Function* func) {
        auto func_type = this->build_function_type(func);
        this->unique(&func_type);
        return func_type;
    }

    void add_slice_attributes (Ast_Slice_Type* slice_type) {
        auto ptr_type = this->get_or_add_pointer_type(slice_type->typed_base);

        auto length_decl = new Ast_Declaration("length", this->type_u64);
        auto data_decl = new Ast_Declaration("data", ptr_type);

        slice_type->scope.add(length_decl);
        slice_type->scope.add(data_decl);
    }

	Ast_Function_Type* build_function_type (Ast_Function* func) {
		auto func_type = new Ast_Function_Type();
        func_type->inferred_type = this->type_type;

		For (func->arg_scope->statements) {
			assert(it->stm_type == AST_STATEMENT_DECLARATION);
			auto decl = static_cast<Ast_Declaration*>(it);
			func_type->arg_types.push(decl->type);
		}

		if (func->ret_scope->statements.size > 0) {
			if (func->ret_scope->statements.size > 1) {
				auto tuple_type = new Ast_Tuple_Type();

				For2 (func->ret_scope->statements, stm) {
					assert(stm->stm_type == AST_STATEMENT_DECLARATION);
					auto decl = static_cast<Ast_Declaration*>(stm);
					tuple_type->types.push(decl->type);
				}

				func_type->ret_type = tuple_type;
			} else {
				auto stm = func->ret_scope->statements[0];
				assert(stm->stm_type == AST_STATEMENT_DECLARATION);
				auto decl = static_cast<Ast_Declaration*>(stm);
				func_type->ret_type = decl->type;
			}
		} else func_type->ret_type = this->type_void;

		return func_type;
	}

    void compute_type_name_if_needed (Ast_Type* type) {
        if (type->name) return;

		switch (type->typedef_type) {
	        case AST_TYPEDEF_STRUCT: {
				auto _struct = static_cast<Ast_Struct_Type*>(type);
				if (_struct->is_slice) {
					auto slice = static_cast<Ast_Slice_Type*>(type);

                    assert(slice->base->exp_type == AST_EXPRESSION_TYPE);
                    this->compute_type_name_if_needed(slice->typed_base);

                    memset(this->type_name_buffer, '\0', MAX_TYPE_NAME_LENGTH);
                    sprintf_s(this->type_name_buffer, strlen(slice->typed_base->name) + 3,
                        "[]%s", slice->typed_base->name);

                    slice->name = _strdup(this->type_name_buffer);
				}
				break;
			}
	        case AST_TYPEDEF_POINTER: {
	            auto _ptr = static_cast<Ast_Pointer_Type*>(type);

                assert(_ptr->base->exp_type == AST_EXPRESSION_TYPE);
                this->compute_type_name_if_needed(_ptr->typed_base);

                memset(this->type_name_buffer, '\0', MAX_TYPE_NAME_LENGTH);
                sprintf_s(this->type_name_buffer, strlen(_ptr->typed_base->name) + 2,
                    "*%s", _ptr->typed_base->name);

                _ptr->name = _strdup(this->type_name_buffer);

				break;
	        }
	        case AST_TYPEDEF_ARRAY: {
	            auto _arr = static_cast<Ast_Array_Type*>(type);
                assert(_arr->length_uint > 0);

                assert(_arr->base->exp_type == AST_EXPRESSION_TYPE);
                this->compute_type_name_if_needed(_arr->typed_base);

                memset(this->type_name_buffer, '\0', MAX_TYPE_NAME_LENGTH);
                sprintf_s(this->type_name_buffer, strlen(_arr->typed_base->name) + 23,
                    "[%lld]%s", _arr->length_uint, _arr->typed_base->name);

                _arr->name = _strdup(this->type_name_buffer);

				break;
	        }
	        case AST_TYPEDEF_SLICE: {
	            auto slice = static_cast<Ast_Slice_Type*>(type);

                assert(slice->base->exp_type == AST_EXPRESSION_TYPE);
                this->compute_type_name_if_needed(slice->typed_base);

                memset(this->type_name_buffer, '\0', MAX_TYPE_NAME_LENGTH);
                sprintf_s(this->type_name_buffer, strlen(slice->typed_base->name) + 3,
                    "[]%s", slice->typed_base->name);

                slice->name = _strdup(this->type_name_buffer);

				break;
	        }
	        case AST_TYPEDEF_TUPLE: {
	            auto tuple = static_cast<Ast_Tuple_Type*>(type);
                assert(tuple->types.size > 0);

                auto exp = tuple->types[0];
                assert(exp->exp_type == AST_EXPRESSION_TYPE);
                auto child_type = static_cast<Ast_Type*>(exp);
                this->compute_type_name_if_needed(child_type);

                memset(this->type_name_buffer, '\0', MAX_TYPE_NAME_LENGTH);
                sprintf_s(this->type_name_buffer, strlen(child_type->name) + 2,
                    "<%s", child_type->name);

                for (size_t i = 1; i < tuple->types.size; i++) {
                    strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, ", ");

                    exp = tuple->types[i];
                    assert(exp->exp_type == AST_EXPRESSION_TYPE);
                    child_type = static_cast<Ast_Type*>(exp);
                    this->compute_type_name_if_needed(child_type);

                    strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, child_type->name);
                }
                strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, ">");

                tuple->name = _strdup(this->type_name_buffer);

				break;
	        }
	        case AST_TYPEDEF_FUNCTION: {
	            auto _func = static_cast<Ast_Function_Type*>(type);

                memset(this->type_name_buffer, '\0', MAX_TYPE_NAME_LENGTH);
                strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, "fn (");
                if (_func->arg_types.size > 0) {
                    auto arg_type = _func->arg_types[0];
                    assert(arg_type->exp_type == AST_EXPRESSION_TYPE);

                    auto typed_type = static_cast<Ast_Type*>(arg_type);
                    this->compute_type_name_if_needed(typed_type);
                    strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, typed_type->name);
                    for (size_t i = 1; i < _func->arg_types.size; i++) {
                        strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, ", ");

                        arg_type = _func->arg_types[i];
                        assert(arg_type->exp_type == AST_EXPRESSION_TYPE);

                        typed_type = static_cast<Ast_Type*>(arg_type);
                        this->compute_type_name_if_needed(typed_type);
                        strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, typed_type->name);
                    }
                }
                strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, ") -> ");

                assert(_func->ret_type->exp_type == AST_EXPRESSION_TYPE);
                auto typed_ret_type = static_cast<Ast_Type*>(_func->ret_type);
                this->compute_type_name_if_needed(typed_ret_type);
                strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, typed_ret_type->name);

                _func->name = _strdup(this->type_name_buffer);

                break;
	        }
	        default: assert(false);
	    }
    }
};
