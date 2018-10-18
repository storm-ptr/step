// Andrew Naplavkov

#ifndef STEP_TEST_UTILITY_HPP
#define STEP_TEST_UTILITY_HPP

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>

struct case_insensitive_hash {
    template <typename T>
    size_t operator()(T that) const
    {
        return std::hash<T>{}(tolower(that));
    }
};

struct case_insensitive_equal_to {
    template <typename T>
    bool operator()(T lhs, T rhs) const
    {
        return tolower(lhs) == tolower(rhs);
    }
};

struct case_insensitive_less {
    template <typename T>
    bool operator()(T lhs, T rhs) const
    {
        return tolower(lhs) < tolower(rhs);
    }
};

template <typename Key, typename T>
using case_insensitive_unordered_map = std::
    unordered_map<Key, T, case_insensitive_hash, case_insensitive_equal_to>;

inline std::string make_random_string(size_t len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    static std::mt19937 generator{std::random_device{}()};
    static std::uniform_int_distribution<size_t> distribution{
        0, sizeof(alphanum) - 2};
    std::string result(len, '\0');
    std::generate(result.begin(), result.end(), [&] {
        return alphanum[distribution(generator)];
    });
    return result;
}

template <class T>
struct manipulator {
    static constexpr int width = 12;

    const T& val;
    std::ios_base& (*align)(std::ios_base&);

    friend std::ostream& operator<<(std::ostream& os, const manipulator& manip)
    {
        return os << std::setw(width) << manip.align << manip.val;
    }
};

template <class T>
auto left(const T& val)
{
    return manipulator<T>{val, std::left};
}

template <class T>
auto right(const T& val)
{
    return manipulator<T>{val, std::right};
}

#endif  // STEP_TEST_UTILITY_HPP
