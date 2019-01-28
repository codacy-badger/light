#pragma once

struct Event {
    size_t id;
    void* data;

    Event (size_t id, void* data = 0) {
        this->id = id;
        this->data = data;
    }
};
