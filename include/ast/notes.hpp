#pragma once

#include "ast/ast.hpp"

#include <vector>

#define GLOBAL_NOTE_END "end"

struct Notes {
    vector<Ast_Note*> global;
	vector<Ast_Note*> notes;

    void push (Ast_Note* note) {
        if (note->is_global) {
            if (strcmp(note->name, GLOBAL_NOTE_END) == 0) {
                this->global.clear();
                delete note;
            } else {
                this->global.push_back(note);
            }
        } else this->notes.push_back(note);
    }

    void push_global_into (Ast_Statement* stm) {
		if (this->global.size() && stm != NULL) {
			stm->notes.insert(stm->notes.end(), this->global.begin(), this->global.end());
		}
    }

    void push_into (Ast_Statement* stm) {
        if (stm != NULL) {
            stm->notes.insert(stm->notes.end(), this->notes.begin(), this->notes.end());
            this->notes.clear();
        }
    }

    void clear_global () {
        this->global.clear();
    }
};
