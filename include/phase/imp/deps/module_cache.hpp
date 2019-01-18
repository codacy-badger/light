#pragma once

#include "phase/phase.hpp"
#include "phase/ast_navigator.hpp"
#include "compiler_events.hpp"
#include "util/string_map.hpp"

#include <vector>

struct Module_Cache : Phase {
    String_Map<Ast_Scope*> cache;
    std::vector<const char*> in_progress;

    Module_Cache() : Phase("Module Cache", CE_IMPORT_MODULE) {
        this->bind(CE_MODULE_READY);
    }

    void on_event (Event event) {
        switch (event.id) {
            case CE_IMPORT_MODULE: {
                auto absolute_path = reinterpret_cast<char*>(event.data);

                if (!this->cache.contains(absolute_path)) {
                    if (!this->is_in_progress(absolute_path)) {
                        this->add_in_progress(absolute_path);
                        this->push(absolute_path);
                        Logger::verbose("Module cache MISS: '%s'", absolute_path);
                    } else Logger::verbose("Module cache IN PROGRESS: '%s'", absolute_path);
                } else Logger::verbose("Module cache HIT: '%s'", absolute_path);
                break;
            }
            case CE_MODULE_READY: {
                this->on_module_ready(event.data);
                break;
            }
        }
    }

    void on_module_ready (void* data) {
        auto global_scope = reinterpret_cast<Ast_Scope*>(data);

        this->remove_in_progress(global_scope->get_absolute_path());
    }

    std::vector<const char*>::iterator find_in_progress (const char* absolute_path) {
        auto it = this->in_progress.begin();
        while (it != this->in_progress.end()) {
            if (strcmp(*it, absolute_path) == 0) {
                return it;
            } else it++;
        }
        return it;
    }

    bool is_in_progress (const char* absolute_path) {
        return this->find_in_progress(absolute_path) != this->in_progress.end();
    }

    void add_in_progress (const char* absolute_path) {
        this->in_progress.push_back(absolute_path);
    }

    void remove_in_progress (const char* absolute_path) {
        auto it = this->find_in_progress(absolute_path);
        if (it != this->in_progress.end()) {
            this->in_progress.erase(it);
        }
    }
};
