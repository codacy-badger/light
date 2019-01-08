#include "modules.hpp"

#include "compiler.hpp"
#include "platform.hpp"

#include "pipeline/pipes/external_modules.hpp"
#include "pipeline/pipes/symbol_resolution.hpp"
#include "pipeline/pipes/constant_propagation.hpp"
#include "pipeline/pipes/constant_folding.hpp"
#include "pipeline/pipes/cast_strings.hpp"
#include "pipeline/pipes/cast_arrays.hpp"
#include "pipeline/pipes/cast_anys.hpp"
#include "pipeline/pipes/type_checking.hpp"
#include "pipeline/pipes/foreign_function.hpp"
#include "pipeline/pipes/static_if.hpp"
#include "pipeline/pipes/call_arguments.hpp"

#include "pipeline/pipes/register_allocator.hpp"
#include "pipeline/pipes/bytecode_generator.hpp"
#include "pipeline/pipes/run_directive.hpp"

Modules::Modules (Compiler* compiler) {
    this->pipeline
        ->pipe(new External_Modules())

        ->pipe(new Foreign_Function())
        ->pipe(new Symbol_Resolution())
        ->pipe(new Constant_Propagation())
        ->pipe(new Static_If())
        ->pipe(new Cast_Strings())
        ->pipe(new Type_Checking())
        ->pipe(new Cast_Arrays())
        ->pipe(new Cast_Anys())
        ->pipe(new Constant_Folding())
        ->pipe(new Call_Arguments())

        ->pipe(new Register_Allocator(INTERP_REGISTER_COUNT))
        ->pipe(new Bytecode_Generator())
        ->pipe(new Run_Directive(compiler->interp));

    //Events::add_observer(CE_MODULE_PARSED, &Modules::handle_module_parsed, this);
    Events::add_observer(CE_IMPORT_MODULE, &Modules::on_import_module, this);
    Events::add_observer(CE_MODULE_READY, &Modules::on_module_ready, this);
}

void Modules::on_import_module (void* data) {
    auto absolute_path = reinterpret_cast<char*>(data);

    if (!this->is_module_cached(absolute_path)) {
        Events::trigger(CE_IMPORT_NEW_MODULE, absolute_path);
    } else {
        auto cached_module = this->cache[absolute_path];
        printf("Found cached module: '%s'\n", cached_module->absolute_path);
        Events::trigger(CE_MODULE_READY, cached_module);
    }
}

void Modules::on_module_ready (void* data) {
    auto module = reinterpret_cast<Module*>(data);

    printf("Module '%s' is ready!\n", module->absolute_path);
    this->cache[module->absolute_path] = module;
}

void Modules::handle_module_parsed (void* data) {
    auto module = reinterpret_cast<Module*>(data);

    this->pipeline->process(module->global_scope);

    Events::trigger(CE_MODULE_READY, module);
}

bool Modules::is_module_cached (const char* absolute_path) {
    auto it = this->cache.find(absolute_path);
    return it != this->cache.end();
}

void Modules::print_metrics (double userInterval) {
	printf("\n");
    this->pipeline->print_metrics(userInterval);
}
