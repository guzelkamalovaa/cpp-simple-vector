#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <new>

#ifndef ARRAY_PTR_H
#define ARRAY_PTR_H

struct ReserveType {
    size_t capacity;
    explicit ReserveType(size_t cap) : capacity(cap) {}
};

inline ReserveType Reserve(size_t cap) {
    return ReserveType(cap);
}

#endif