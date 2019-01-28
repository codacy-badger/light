#pragma once

struct Type_Conversion : Phase, Ast_Navigator {

    Type_Conversion() : Phase("Type Conversion", CE_MODULE_TYPE_CONVERSION) { /* empty */ }

    void on_event (Event event) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(event.data);

        Ast_Navigator::ast_handle(global_scope);

        this->push(global_scope);
    }

    void match_types (Ast_Expression** exp_ptr, Ast_Type* given, Ast_Type* expected) {
        std::vector<Ast_Statement*> to_prepend;
        if (!this->check_type_match(&to_prepend, exp_ptr, given, expected)) {
            Logger::error_and_stop(*exp_ptr, "Type mismatch: given '%s' but '%s' was expected",
                given->name, expected->name);
        } else this->prepend_statements(&to_prepend);
    }

    void match_types (Ast_Expression** exp_ptr1, Ast_Expression** exp_ptr2) {
        std::vector<Ast_Statement*> to_prepend;
        if (!this->check_type_match(&to_prepend, exp_ptr1, (*exp_ptr2)->inferred_type)
                && !this->check_type_match(&to_prepend, exp_ptr2, (*exp_ptr1)->inferred_type)) {
            Logger::error_and_stop(*exp_ptr1, "Type mismatch: '%s' and '%s' are not compatible",
                (*exp_ptr1)->inferred_type->name, (*exp_ptr2)->inferred_type->name);
        } else this->prepend_statements(&to_prepend);
    }

    void match_types (Ast_Expression** exp_ptr, Ast_Type* expected) {
        this->match_types(exp_ptr, (*exp_ptr)->inferred_type, expected);
    }

    bool check_type_match (std::vector<Ast_Statement*>* to_prepend, Ast_Expression** exp_ptr, Ast_Type* expected) {
        return this->check_type_match(to_prepend, exp_ptr, (*exp_ptr)->inferred_type, expected);
    }

    bool check_type_match (std::vector<Ast_Statement*>* to_prepend, Ast_Expression** exp_ptr, Ast_Type* given, Ast_Type* expected) {
        return (given == expected) || this->try_convert(to_prepend, exp_ptr, given, expected);
    }

	void ast_handle (Ast_Declaration* decl) {
		if (decl->expression && decl->type) {
			auto decl_type_inst = static_cast<Ast_Type*>(decl->type);
            this->match_types(&decl->expression, decl_type_inst);
		}
        Ast_Navigator::ast_handle(decl);
	}

	void ast_handle (Ast_If* _if) {
        Ast_Navigator::ast_handle(_if);
        this->match_types(&_if->condition, Types::type_bool);
	}

	void ast_handle (Ast_While* _while) {
        Ast_Navigator::ast_handle(_while);
        this->match_types(&_while->condition, Types::type_bool);
	}

	void ast_handle (Ast_Return* ret) {
		auto func = this->current_scope->get_parent_function();
		auto ret_type_def = static_cast<Ast_Type*>(func->type->ret_type);
		if (func && ret->expression) {
			if (func->type->ret_type != Types::type_void) {
        		Ast_Navigator::ast_handle(ret);
				this->match_types(&ret->expression, ret_type_def);
			}
		}
	}

	void ast_handle (Ast_Function_Call* call) {
	    if (call->func->inferred_type->typedef_type != AST_TYPEDEF_FUNCTION) {
			Logger::error_and_stop(call, "Function calls can only be performed with functions types");
		}

		auto func_type = static_cast<Ast_Function_Type*>(call->func->inferred_type);

		for (int i = 0; i < call->arguments->unnamed.size(); i++) {
			if (i >= func_type->arg_decls.size()) break;

			Ast_Navigator::ast_handle(call->arguments->unnamed[i]);
			auto param_exp = call->arguments->unnamed[i];
			assert(param_exp->inferred_type);

			auto arg_type = static_cast<Ast_Type*>(func_type->arg_decls[i]->type);
            this->match_types(&call->arguments->unnamed[i], arg_type);
		}
	}

	void ast_handle (Ast_Binary* binop) {
	    Ast_Navigator::ast_handle(binop);

        if (binop->binary_op == AST_BINARY_ATTRIBUTE) {
            return;
        } else if (binop->binary_op == AST_BINARY_SUBSCRIPT) {
            this->match_types(&binop->rhs, Types::type_u64);
		} else if (binop->binary_op == AST_BINARY_ASSIGN) {
            this->match_types(&binop->rhs, binop->lhs->inferred_type);
		} else this->match_types(&binop->lhs, &binop->rhs);
	}

    bool try_convert (std::vector<Ast_Statement*>* to_prepend, Ast_Expression** exp_ptr,
            Ast_Type* type_from, Ast_Type* type_to) {
        return this->try_implicid_cast(exp_ptr, type_from, type_to)
            || this->try_coercion(to_prepend, exp_ptr, type_from, type_to);
    }

    bool try_implicid_cast (Ast_Expression** exp_ptr,
            Ast_Type* type_from, Ast_Type* type_to) {
        if (type_from->is_primitive && type_to->is_primitive) {
            if (type_to == Types::type_bool) {
                auto cast = new Ast_Cast((*exp_ptr), type_to);
                cast->location = (*exp_ptr)->location;
                cast->inferred_type = type_to;
                (*exp_ptr) = cast;
                return true;
            } else if (type_from->is_signed == type_to->is_signed) {
                if (type_to->byte_size >= type_from->byte_size) {
                    auto cast = new Ast_Cast((*exp_ptr), type_to);
                    cast->location = (*exp_ptr)->location;
                    cast->inferred_type = type_to;
                    (*exp_ptr) = cast;
                    return true;
                }
            } else if (!type_from->is_signed && type_to->is_signed) {
                if (type_to->byte_size > type_from->byte_size) {
                    auto cast = new Ast_Cast((*exp_ptr), type_to);
                    cast->location = (*exp_ptr)->location;
                    cast->inferred_type = type_to;
                    (*exp_ptr) = cast;
                    return true;
                }
            }
        }
        return false;
    }

    bool try_implicid_cast (Ast_Expression** exp_ptr, Ast_Type* type_to) {
        return this->try_implicid_cast(exp_ptr, (*exp_ptr)->inferred_type, type_to);
    }

    bool try_coercion (std::vector<Ast_Statement*>* to_prepend, Ast_Expression** exp_ptr,
            Ast_Type* type_from, Ast_Type* type_to) {
            if (type_to == Types::type_any) {
                this->coerce_to_any(to_prepend, exp_ptr);
                return true;
            } else if (type_from->typedef_type == AST_TYPEDEF_ARRAY) {
                auto type_from_array = static_cast<Ast_Array_Type*>(type_from);
                if (type_from_array->base == Types::type_byte && type_to == Types::type_string) {
                    this->coerce_char_array_to_string(to_prepend, exp_ptr);
                    return true;
                } else if (type_to->typedef_type == AST_TYPEDEF_STRUCT) {
                    auto type_to_struct = static_cast<Ast_Struct_Type*>(type_to);
                    if (type_to_struct->is_slice) {
                        auto type_to_slice = static_cast<Ast_Slice_Type*>(type_to);
                        if (type_to_slice->get_typed_base() == type_from_array->base) {
                            this->coerce_array_to_slice(to_prepend, exp_ptr, type_to_slice);
                            return true;
                        }
                    }
                }
            }
        return false;
    }

    void coerce_char_array_to_string (std::vector<Ast_Statement*>* to_prepend, Ast_Expression** exp_ptr) {
        auto str = (*exp_ptr);

        assert(str->inferred_type->typedef_type == AST_TYPEDEF_ARRAY);
        auto arr_type = static_cast<Ast_Array_Type*>(str->inferred_type);

        auto tmp_name = (char*) malloc(TMP_NAME_SIZE + 1);
        static size_t string_coercion_count = 0;
        sprintf_s(tmp_name, TMP_NAME_SIZE, "$str$%zd", string_coercion_count++);

        auto string_data_attr_decl = Types::type_string->find_attribute("data");
        assert(string_data_attr_decl != NULL);
        assert(string_data_attr_decl->type->exp_type == AST_EXPRESSION_TYPE_INSTANCE);

        // $tmp : string;
        auto string_declaration = Ast_Factory::declaration(str->location, tmp_name, Types::type_string, NULL, false);
        string_declaration->scope = this->current_scope;
        to_prepend->push_back(string_declaration);

        // $tmp.length = array.length;
        auto tmp_ident1 = Ast_Factory::ident(str->location, tmp_name, string_declaration, Types::type_string);
        auto tmp_length = Ast_Factory::attr(tmp_ident1, "length");
        auto str_length = Ast_Factory::literal_array_length(str->location, arr_type);
        auto assign_length = Ast_Factory::assign(tmp_length, str_length);
        to_prepend->push_back(assign_length);

        // $tmp.data = array.data;
        auto tmp_ident2 = Ast_Factory::ident(str->location, tmp_name, string_declaration, Types::type_string);
        auto tmp_data = Ast_Factory::attr(tmp_ident2, "data");
        auto str_data = Ast_Factory::ref(str, tmp_data->inferred_type);
        auto assign_data = Ast_Factory::assign(tmp_data, str_data);
        to_prepend->push_back(assign_data);

        (*exp_ptr) = Ast_Factory::ident(str->location, tmp_name, string_declaration, Types::type_string);
    }

    void coerce_to_any (std::vector<Ast_Statement*>* to_prepend, Ast_Expression** exp_ptr) {
        auto val = (*exp_ptr);

        auto tmp_name = (char*) malloc(TMP_NAME_SIZE + 1);
        static size_t any_coercion_count = 0;
        sprintf_s(tmp_name, TMP_NAME_SIZE, "$any$%zd", any_coercion_count++);

        // $tmp : any;
        auto any_declaration = Ast_Factory::declaration(val->location, tmp_name, Types::type_any, NULL, false);
        any_declaration->scope = this->current_scope;
        to_prepend->push_back(any_declaration);

        // $tmp.type = #;
        auto tmp_ident1 = Ast_Factory::ident(val->location, tmp_name, any_declaration, Types::type_any);
        auto tmp_type = Ast_Factory::attr(tmp_ident1, "type");
        auto val_type = Ast_Factory::literal(val->location, (uint64_t)Types::get_internal_type(val->inferred_type));
        auto assign_type = Ast_Factory::assign(tmp_type, val_type);
        to_prepend->push_back(assign_type);

        // $tmp.value = ID;
        auto tmp_ident2 = Ast_Factory::ident(val->location, tmp_name, any_declaration, Types::type_any);
        auto value_ref_type = static_cast<Ast_Type*>(Types::type_any->find_attribute("value")->type);
        auto tmp_value = Ast_Factory::attr(tmp_ident2, "value");
        auto val_value = Ast_Factory::ref(val, value_ref_type);
        auto assign_value = Ast_Factory::assign(tmp_value, val_value);
        to_prepend->push_back(assign_value);

        (*exp_ptr) = Ast_Factory::ident(val->location, tmp_name, any_declaration, Types::type_any);
    }

    void coerce_array_to_slice (std::vector<Ast_Statement*>* to_prepend,
            Ast_Expression** exp_ptr, Ast_Slice_Type* target) {
        auto exp = (*exp_ptr);

        assert(exp->inferred_type->typedef_type == AST_TYPEDEF_ARRAY);
        auto arr_type = static_cast<Ast_Array_Type*>(exp->inferred_type);

        auto tmp_name = (char*) malloc(TMP_NAME_SIZE + 1);
        static size_t slice_coercion_count = 0;
        sprintf_s(tmp_name, TMP_NAME_SIZE, "$sli$%zd", slice_coercion_count++);

        // $tmp : slice;
        auto slice_declaration = Ast_Factory::declaration(exp->location, tmp_name, target, NULL, false);
        slice_declaration->scope = this->current_scope;
        to_prepend->push_back(slice_declaration);

        // $tmp.length = array.length;
        auto tmp_ident1 = Ast_Factory::ident(exp->location, tmp_name, slice_declaration, target);
        auto tmp_length = Ast_Factory::attr(tmp_ident1, "length");
        auto str_length = Ast_Factory::literal_array_length(exp->location, arr_type);
        auto assign_length = Ast_Factory::assign(tmp_length, str_length);
        to_prepend->push_back(assign_length);

        // $tmp.data = array.data;
        auto tmp_ident2 = Ast_Factory::ident(exp->location, tmp_name, slice_declaration, target);
        auto tmp_data = Ast_Factory::attr(tmp_ident2, "data");
        auto exp_data = Ast_Factory::ref(exp, tmp_data->inferred_type);
        auto assign_data = Ast_Factory::assign(tmp_data, exp_data);
        to_prepend->push_back(assign_data);

        (*exp_ptr) = Ast_Factory::ident(exp->location, tmp_name, slice_declaration, target);
    }
};
