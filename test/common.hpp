// Andrew Naplavkov

#ifndef STEP_TEST_COMMON_HPP
#define STEP_TEST_COMMON_HPP

#include <cctype>
#include <type_traits>

inline bool iequals(char lhs, char rhs)
{
    return tolower(lhs) == tolower(rhs);
}

/// @see https://en.wikipedia.org/wiki/Kahan_summation_algorithm
template <typename T>
struct kahan_plus {
    static_assert(std::is_floating_point_v<T>);

    mutable T compensation{};

    T operator()(T lhs, T rhs) const
    {
        rhs -= compensation;
        T result = lhs + rhs;
        compensation = result - lhs - rhs;
        return result;
    }
};

#endif  // STEP_TEST_COMMON_HPP
