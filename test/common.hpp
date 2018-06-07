// Andrew Naplavkov

#ifndef STEP_TEST_COMMON_HPP
#define STEP_TEST_COMMON_HPP

#include <cctype>

inline bool iequals(char lhs, char rhs)
{
    return tolower(lhs) == tolower(rhs);
}

#endif  // STEP_TEST_COMMON_HPP
