#pragma once

struct String_Vector : std::vector<const char*> {
    bool contains (const char* another) {
        for (size_t i = 0; i < this->size(); i++) {
            auto item = (*this)[i];
            if (strcmp(item, another) == 0) {
                return true;
            }
        }
    }

    bool remove (const char* to_remove) {
        size_t i = 0;
        while (i < this->size()) {
            auto item = (*this)[i];
            if (strcmp(item, to_remove) == 0) {
                this->erase(this->begin() + i);
                return true;
            } else i++;
        }
        return false;
    }
};
