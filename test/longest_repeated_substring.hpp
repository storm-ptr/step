// Andrew Naplavkov

#ifndef STEP_TEST_LONGEST_REPEATED_SUBSTRING_HPP
#define STEP_TEST_LONGEST_REPEATED_SUBSTRING_HPP

#include <map>
#include <step/longest_repeated_substring.hpp>
#include <string>
#include <string_view>

TEST_CASE("longest_repeated_substring")
{
    struct {
        std::string_view str;
        std::string_view expected;
    } CASES[] = {
        {"GEEKSFORGEEKS$", "GEEKS"},
        {"AAAAAAAAAA$", "AAAAAAAAA"},
        {"ABCDEFG$", ""},
        {"ABABABA$", "ABABA"},
        {"ATCGATCGA$", "ATCGA"},
        {"banana$", "ana"},
        {"abcpqrabpqpq$", "ab"},
        {"pqrpqpqabab$", "ab"},
        {"ATCGATCGA$", "ATCGA"},
    };
    for (auto& [str, expected] : CASES) {
        auto r = step::longest_repeated_substring::find<std::map>(str);
        CHECK(expected == std::string(r.first, r.second));
    }
}

#endif  // STEP_TEST_LONGEST_REPEATED_SUBSTRING_HPP
