#include "build_context.hpp"

#include "workspace.hpp"

#include "parser/parser.hpp"

#include "pipeline/service/modules.hpp"
#include "pipeline/service/type_inferrer.hpp"
#include "pipeline/service/type_table.hpp"
#include "pipeline/service/type_caster.hpp"

// DEBUG INCLUDE
#include "ast/printer.hpp"

#include <stdarg.h>

#define VA_PRINT(print_function) va_list args;      \
    va_start(args, format);                         \
    print_function(format, args);                   \
    va_end(args)

#define VA_PRINT_AST(print_function, node) va_list args;       \
    va_start(args, format);                                    \
    print_function(format, args);                              \
    this->print_location(node->path, node->line);              \
    va_end(args)

void debug_v (const char* format, va_list args);
void error_v (const char* format, va_list args);

void Build_Context::init (Workspace* w) {
    this->workspace = w;

    this->arena = new Memory_Arena();

    this->parser = new Parser();

    this->modules = new Modules();
    this->type_inferrer = new Type_Inferrer();
    this->type_table = new Type_Table();
    this->type_caster = new Type_Caster();

    this->printer = new Ast_Printer();

    this->type_table->init(this);
    this->type_inferrer->init(this);
    this->type_caster->init(this);
    this->parser->init(this);
    this->modules->init(this);

    os_get_current_directory(this->base_path);
}

void Build_Context::debug (const char* format, ...) {
    VA_PRINT(debug_v);
}

void Build_Context::debug (Ast* node, const char* format, ...) {
    assert(node);
    VA_PRINT_AST(debug_v, node);
}

void Build_Context::error (const char* format, ...) {
    VA_PRINT(error_v);
}

void Build_Context::error (Ast* node, const char* format, ...) {
    assert(node);
    VA_PRINT_AST(error_v, node);
}

void Build_Context::shutdown () {
    if (!this->workspace->keep_going) return;
    this->workspace->stop_with_errors();
}

void Build_Context::print_location (const char* path, size_t line) {
    if (!this->workspace->keep_going) return;

    printf("\t@ %s(%zd)\n", path, line);
}

void Build_Context::debug_v (const char* format, va_list args) {
    if (!this->workspace->keep_going) return;

    printf("[DEBUG] ");
    vprintf(format, args);
    printf("\n");
}

void Build_Context::error_v (const char* format, va_list args) {
    if (!this->workspace->keep_going) return;

    this->has_error = true;

    printf("[ERROR] ");
    vprintf(format, args);
    printf("\n");
}
