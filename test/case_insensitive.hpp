// Andrew Naplavkov

#ifndef STEP_CASE_INSENSITIVE_HPP
#define STEP_CASE_INSENSITIVE_HPP

#include <cctype>
#include <functional>
#include <string>
#include <unordered_map>

namespace step::case_insensitive {

struct hash {
    template <class T>
    size_t operator()(T that) const
    {
        return std::hash<T>{}(tolower(that));
    }
};

struct equal_to {
    template <class T>
    bool operator()(T lhs, T rhs) const
    {
        return tolower(lhs) == tolower(rhs);
    }
};

struct less {
    template <class T>
    bool operator()(T lhs, T rhs) const
    {
        return tolower(lhs) < tolower(rhs);
    }
};

template <class Key, class T>
using unordered_map = std::unordered_map<Key, T, hash, equal_to>;

}  // namespace step::case_insensitive

#endif  // STEP_CASE_INSENSITIVE_HPP
