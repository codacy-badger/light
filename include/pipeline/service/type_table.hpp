#pragma once

#include "ast/nodes.hpp"
#include "ast/types.hpp"

#include "utils/array.hpp"

#define MAX_TYPE_NAME_LENGTH 1024

struct Type_Table {
    Build_Context* context = NULL;

    Array<Ast_Type*> global_type_table = Array<Ast_Type*>(16);

    char type_name_buffer[MAX_TYPE_NAME_LENGTH];

    void init (Build_Context* c) { this->context = c; }

    void unique (Ast_Type** type_ptr) {
        auto type = (*type_ptr);

        if (type->guid > 0) return;

        switch (type->typedef_type) {
            case AST_TYPEDEF_STRUCT: {
                this->unique(reinterpret_cast<Ast_Struct_Type**>(type_ptr));
                assert(type->guid > 0);
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
            default: assert(false);
        }
    }

    void unique (Ast_Struct_Type** struct_ptr) {
        auto _struct = (*struct_ptr);

        if (_struct->guid > 0) return;

        _struct->guid = this->global_type_table.size + 1;
        this->global_type_table.push(_struct);
        this->compute_type_name_if_needed(_struct);
    }

    void unique (Ast_Pointer_Type** ptr_type_ptr) {
        auto ptr_type = (*ptr_type_ptr);

        if (ptr_type->guid > 0) return;

        assert(ptr_type->base->exp_type == AST_EXPRESSION_TYPE);
        this->unique(&ptr_type->typed_base);

        For2 (this->global_type_table, type) {
            if (type->typedef_type == AST_TYPEDEF_POINTER) {
                auto uniqued_ptr_type = static_cast<Ast_Pointer_Type*>(type);
                assert(uniqued_ptr_type->base->exp_type == AST_EXPRESSION_TYPE);

                if (uniqued_ptr_type->typed_base->guid == ptr_type->typed_base->guid) {
                    (*ptr_type_ptr) = uniqued_ptr_type;
                    return;
                }
            }
        }

        ptr_type->guid = this->global_type_table.size + 1;
        this->global_type_table.push(ptr_type);
        this->compute_type_name_if_needed(ptr_type);
    }

    void unique (Ast_Array_Type** array_type_ptr) {
        auto array_type = (*array_type_ptr);

        if (array_type->guid > 0) return;

        assert(array_type->base->exp_type == AST_EXPRESSION_TYPE);
        this->unique(&array_type->typed_base);

        For2 (this->global_type_table, type) {
            if (type->typedef_type == AST_TYPEDEF_ARRAY) {
                auto uniqued_array_type = static_cast<Ast_Array_Type*>(type);
                assert(uniqued_array_type->base->exp_type == AST_EXPRESSION_TYPE);

                if (uniqued_array_type->typed_base->guid == array_type->typed_base->guid) {
                    (*array_type_ptr) = uniqued_array_type;
                    return;
                }
            }
        }

        array_type->guid = this->global_type_table.size+ 1;
        this->global_type_table.push(array_type);
        this->compute_type_name_if_needed(array_type);
    }

    void unique (Ast_Slice_Type** slice_type_ptr) {
        auto slice_type = (*slice_type_ptr);

        if (slice_type->guid > 0) return;

        assert(slice_type->get_base()->exp_type == AST_EXPRESSION_TYPE);
        this->unique(slice_type->get_typed_base_ptr());

        For2 (this->global_type_table, type) {
            if (type->typedef_type == AST_TYPEDEF_SLICE) {
                auto uniqued_slice_type = static_cast<Ast_Slice_Type*>(type);
                assert(uniqued_slice_type->get_base()->exp_type == AST_EXPRESSION_TYPE);

                if (uniqued_slice_type->get_typed_base()->guid == slice_type->get_typed_base()->guid) {
                    (*slice_type_ptr) = uniqued_slice_type;
                    return;
                }
            }
        }

        slice_type->guid = this->global_type_table.size+ 1;
        this->global_type_table.push(slice_type);
        this->compute_type_name_if_needed(slice_type);
    }

    void unique (Ast_Function_Type** func_type_ptr) {
        auto func_type = (*func_type_ptr);

        if (func_type->guid > 0) return;

        for (auto& arg_type : func_type->arg_types) {
            assert(arg_type->exp_type == AST_EXPRESSION_TYPE);
            this->unique((Ast_Type**) &arg_type);
        }
        for (auto& ret_type : func_type->ret_types) {
            assert(ret_type->exp_type == AST_EXPRESSION_TYPE);
            this->unique((Ast_Type**) &ret_type);
        }

        For2 (this->global_type_table, type) {
            if (type->typedef_type == AST_TYPEDEF_FUNCTION) {
                auto uniqued_function_type = static_cast<Ast_Function_Type*>(type);

                if (this->types_are_equal(uniqued_function_type, func_type)) {
                    (*func_type_ptr) = uniqued_function_type;
                    return;
                }
            }
        }

        func_type->guid = this->global_type_table.size + 1;
        this->global_type_table.push(func_type);
        this->compute_type_name_if_needed(func_type);
    }

    bool types_are_equal (Ast_Function_Type* func_type1, Ast_Function_Type* func_type2) {
        if (func_type1 == func_type2) return true;

        if (func_type1->arg_types.size() != func_type2->arg_types.size()) return false;
        if (func_type1->ret_types.size() != func_type2->ret_types.size()) return false;

        for (size_t i = 0; i < func_type1->arg_types.size(); i++) {
            auto attr_type1 = func_type1->arg_types[i];
            auto attr_type2 = func_type2->arg_types[i];

            if (attr_type1 != attr_type2) return false;
        }

        for (size_t i = 0; i < func_type1->ret_types.size(); i++) {
            auto ret_type1 = func_type1->ret_types[i];
            auto ret_type2 = func_type2->ret_types[i];

            if (ret_type1 != ret_type2) return false;
        }

        return true;
    }

    Ast_Pointer_Type* get_or_add_pointer_type (Ast_Type* base_type) {
        this->unique(&base_type);

        For2 (this->global_type_table, type) {
            if (type->typedef_type == AST_TYPEDEF_POINTER) {
                auto uniqued_ptr_type = static_cast<Ast_Pointer_Type*>(type);
                assert(uniqued_ptr_type->base->exp_type == AST_EXPRESSION_TYPE);

                if (uniqued_ptr_type->typed_base->guid == base_type->guid) {
                    return uniqued_ptr_type;
                }
            }
        }

        auto ptr_type = new Ast_Pointer_Type(base_type);
        ptr_type->guid = this->global_type_table.size + 1;
        this->global_type_table.push(ptr_type);
        this->compute_type_name_if_needed(ptr_type);
        return ptr_type;
    }

    void compute_type_name_if_needed (Ast_Type* type) {
        if (type->name) return;

		switch (type->typedef_type) {
	        case AST_TYPEDEF_STRUCT: {
				auto _struct = static_cast<Ast_Struct_Type*>(type);
				if (_struct->is_slice) {
					auto slice = static_cast<Ast_Slice_Type*>(type);

                    auto base = slice->get_base();
                    assert(base->exp_type == AST_EXPRESSION_TYPE);
                    auto base_type = static_cast<Ast_Type*>(base);
                    this->compute_type_name_if_needed(base_type);

                    memset(this->type_name_buffer, '\0', MAX_TYPE_NAME_LENGTH);
                    sprintf_s(this->type_name_buffer, strlen(base_type->name) + 3,
                        "[]%s", base_type->name);

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

                assert(slice->get_base()->exp_type == AST_EXPRESSION_TYPE);
                this->compute_type_name_if_needed(slice->get_typed_base());

                memset(this->type_name_buffer, '\0', MAX_TYPE_NAME_LENGTH);
                sprintf_s(this->type_name_buffer, strlen(slice->get_typed_base()->name) + 3,
                    "[]%s", slice->get_typed_base()->name);

                slice->name = _strdup(this->type_name_buffer);

				break;
	        }
	        case AST_TYPEDEF_FUNCTION: {
	            auto _func = static_cast<Ast_Function_Type*>(type);

                memset(this->type_name_buffer, '\0', MAX_TYPE_NAME_LENGTH);
                strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, "fn (");
                if (_func->arg_types.size() > 0) {
                    auto arg_type = _func->arg_types[0];
                    assert(arg_type->exp_type == AST_EXPRESSION_TYPE);

                    auto typed_type = static_cast<Ast_Type*>(arg_type);
                    this->compute_type_name_if_needed(typed_type);
                    strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, typed_type->name);
                    for (size_t i = 1; i < _func->arg_types.size(); i++) {
                        strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, ", ");

                        arg_type = _func->arg_types[i];
                        assert(arg_type->exp_type == AST_EXPRESSION_TYPE);

                        typed_type = static_cast<Ast_Type*>(arg_type);
                        this->compute_type_name_if_needed(typed_type);
                        strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, typed_type->name);
                    }
                }
                strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, ") -> ");

                if (_func->ret_types.size() > 0) {
                    auto arg_type = _func->ret_types[0];
                    assert(arg_type->exp_type == AST_EXPRESSION_TYPE);

                    auto typed_type = static_cast<Ast_Type*>(arg_type);
                    this->compute_type_name_if_needed(typed_type);
                    strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, typed_type->name);
                    for (size_t i = 1; i < _func->ret_types.size(); i++) {
                        strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, ", ");

                        arg_type = _func->ret_types[i];
                        assert(arg_type->exp_type == AST_EXPRESSION_TYPE);

                        typed_type = static_cast<Ast_Type*>(arg_type);
                        this->compute_type_name_if_needed(typed_type);
                        strcat_s(this->type_name_buffer, MAX_TYPE_NAME_LENGTH, typed_type->name);
                    }
                }

                _func->name = _strdup(this->type_name_buffer);

                break;
	        }
	        default: assert(false);
	    }
    }
};
