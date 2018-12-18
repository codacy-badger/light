#include "modules.hpp"

#include "compiler.hpp"
#include "platform.hpp"
#include "lexer/buffer/full_buffer.hpp"

#include "pipeline/pipes/external_modules.hpp"
#include "pipeline/pipes/symbol_resolution.hpp"
#include "pipeline/pipes/constant_folding.hpp"
#include "pipeline/pipes/cast_arrays.hpp"
#include "pipeline/pipes/type_checking.hpp"
#include "pipeline/pipes/foreign_function.hpp"
#include "pipeline/pipes/static_if.hpp"

#include "pipeline/pipes/register_allocator.hpp"
#include "pipeline/pipes/bytecode_generator.hpp"
#include "pipeline/pipes/run_directive.hpp"

#define DECL_TYPE(scope, type) scope->statements.push_back(ast_make_declaration(type->name, type));

#define IS_WINDOWS_LITERAL ast_make_literal(os_get_type() == OS_TYPE_WINDOWS)
#define IS_LINUX_LITERAL ast_make_literal(os_get_type() == OS_TYPE_LINUX)
#define IS_MAC_LITERAL ast_make_literal(os_get_type() == OS_TYPE_MAC)

Modules::Modules (Compiler* compiler) {
    DECL_TYPE(this->internal_scope, Types::type_type);
    DECL_TYPE(this->internal_scope, Types::type_void);
    DECL_TYPE(this->internal_scope, Types::type_bool);
    DECL_TYPE(this->internal_scope, Types::type_s8);
    DECL_TYPE(this->internal_scope, Types::type_s16);
    DECL_TYPE(this->internal_scope, Types::type_s32);
    DECL_TYPE(this->internal_scope, Types::type_s64);
    DECL_TYPE(this->internal_scope, Types::type_u8);
    DECL_TYPE(this->internal_scope, Types::type_u16);
    DECL_TYPE(this->internal_scope, Types::type_u32);
    DECL_TYPE(this->internal_scope, Types::type_u64);
    DECL_TYPE(this->internal_scope, Types::type_f32);
    DECL_TYPE(this->internal_scope, Types::type_f64);

    this->internal_scope->add(ast_make_declaration("OS_WINDOWS", IS_WINDOWS_LITERAL));
    this->internal_scope->add(ast_make_declaration("OS_LINUX", IS_LINUX_LITERAL));
    this->internal_scope->add(ast_make_declaration("OS_MAC", IS_MAC_LITERAL));

    this->pipeline
        ->pipe(new External_Modules())

        ->pipe(new Foreign_Function())
        ->pipe(new Symbol_Resolution())
        ->pipe(new Type_Checking())
        ->pipe(new Cast_Arrays())
        ->pipe(new Constant_Folding())
        ->pipe(new Static_If())

        ->pipe(new Register_Allocator())
        ->pipe(new Bytecode_Generator())
        ->pipe(new Run_Directive(compiler->interp));
}

Ast_Scope* Modules::get_module (Code_Source* source) {
    switch (source->type) {
        case CODE_SOURCE_FILE: {
            auto file_source = static_cast<File_Code_Source*>(source);
            return this->get_module(file_source->absolute_path);
        }
        case CODE_SOURCE_STRING: {
            auto string_source = static_cast<String_Code_Source*>(source);

            auto lexer_buffer = new String_Buffer(string_source->source);
            auto lexer = new Lexer(lexer_buffer);
            auto scope = this->parser->run(lexer, this->internal_scope);

            this->pipeline->process(scope);

            return scope;
        }
        default: return NULL;
    }
}

Ast_Scope* Modules::get_module (char* absolute_path) {
    if (!this->is_module_cached(absolute_path)) {
        return this->load_module(absolute_path);
    } else return this->cache[absolute_path];
}

bool Modules::is_module_cached (char* absolute_path) {
    auto it = this->cache.find(absolute_path);
    return it != this->cache.end();
}

Ast_Scope* Modules::load_module (char* absolute_path) {
    char last_path[MAX_PATH_LENGTH];
    os_get_current_directory(last_path);
    os_set_current_directory_path(absolute_path);

    auto lexer_buffer = new Full_Buffer(absolute_path);
    auto lexer = new Lexer(lexer_buffer);
    auto file_scope = this->parser->run(lexer, this->internal_scope);

    this->pipeline->process(file_scope);
    this->cache[absolute_path] = file_scope;

    os_set_current_directory(last_path);

    return file_scope;
}

bool Modules::free_module (char* absolute_path) {
    if (this->is_module_cached(absolute_path)) {
        this->cache.erase(absolute_path);
        return true;
    }
    return false;
}
