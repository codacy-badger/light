#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

template<typename T>
struct Array {
    size_t size = 0;
    size_t capacity = 0;
    T* data = NULL;

    Array (size_t initial_size = 0) {
        if (initial_size > 0) {
            this->_ensure_size(initial_size);
        }
    }

    void resize (size_t new_size) {
        if (this->capacity == new_size) return;

        this->_ensure_size(new_size);
        this->size = new_size;
    }

    void _ensure_size (size_t new_size) {
        if (this->capacity < new_size) {
            auto new_size_pow2 = this->capacity ? this->capacity * 2 : 1;
            while (new_size_pow2 < new_size) new_size_pow2 *= 2;

            this->data = (T*) realloc(this->data, sizeof(T) * new_size_pow2);
            this->capacity = new_size_pow2;
            if (!this->data) {
                fprintf(stderr, "Error allocating memory for Array\n");
                abort();
            }
        }
    }

    void clear () {
        this->size = 0;
    }

    bool empty () {
        return this->size == 0;
    }

    T get (size_t index) {
        assert(index < this->size);
        return this->data[index];
    }

    T set (size_t index, T value) {
        assert(index < this->size);
        return this->data[index] = value;
    }

    void push (T item) {
        this->insert(this->size, item);
    }

    void push (Array<T> other_array) {
        this->insert(this->size, other_array);
    }

    void insert (size_t at, Array<T> other_array) {
        this->_ensure_size(this->size + other_array.size);

        memcpy(this->data + at, this->data + at + other_array.size, this->size - at);

        for (size_t i = 0; i < other_array.size; i++) {
            this->data[at + i] = other_array[i];
        }
        this->size += other_array.size;
    }

    void insert (size_t at, T item) {
        this->_ensure_size(this->size + 1);

        memcpy(this->data + at, this->data + at + 1, this->size - at);

        this->data[at] = item;
        this->size += 1;
    }

    T pop () {
        auto result = this->data[0];
        memcpy(this->data, this->data + 1, this->size - 1);
        this->size -= 1;
        return result;
    }

    T operator [] (size_t index) const {
        return this->data[index];
    }

    T& operator [] (size_t index) {
        return this->data[index];
    }

    bool contains (T item) {
        for (size_t i = 0; i < this->size; i++) {
            if (this->data[i] == item) {
                return true;
            }
        }
        return false;
    }

    size_t find (T item) {
        for (size_t i = 0; i < this->size; i++) {
            if (this->data[i] == item) {
                return i;
            }
        }
        return SIZE_MAX;
    }

    size_t find (T item, bool (*comparator)(T, T)) {
        for (size_t i = 0; i < this->size; i++) {
            if (comparator(this->data[i], item)) {
                return i;
            }
        }
        return SIZE_MAX;
    }
};

#define For(array) For3(array, it, i)
#define For2(array, it) For3(array, it, i)
#define For3(array, it, i) auto it = array.data ? array[0] : NULL;              \
    for (size_t i = 0; i < array.size; i++, it = array[i])

#define For_Ref(array) For_Ref3(array, it, i)
#define For_Ref2(array, it) For_Ref3(array, it, i)
#define For_Ref3(array, it, i) auto it = array.data ? &(array[0]) : NULL;              \
    for (size_t i = 0; i < array.size; i++, it = &(array[i]))
