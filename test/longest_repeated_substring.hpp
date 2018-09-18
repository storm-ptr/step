// Andrew Naplavkov

#ifndef STEP_TEST_LONGEST_REPEATED_SUBSTRING_HPP
#define STEP_TEST_LONGEST_REPEATED_SUBSTRING_HPP

#include <map>
#include <step/longest_repeated_substring.hpp>
#include <step/test/utility.hpp>
#include <string_view>

TEST_CASE("longest_repeated_substring_hello_world")
{
    auto range = step::longest_repeated_substring::find(
        "the longest substring of a string that occurs at least twice");
    CHECK("string " == std::string(range.first, range.second));
}

TEST_CASE("longest_repeated_substring_find")
{
    struct {
        std::string_view str;
        std::string_view expected;
    } tests[] = {
        {"GEEKSFORGEEKS$", "GEEKS"},
        {"AAAAAAAAAA$", "AAAAAAAAA"},
        {"ABCDEFG$", ""},
        {"ABABABA$", "ABABA"},
        {"ATCGATCGA$", "ATCGA"},
        {"banana$", "ana"},
        {"abcpqrabpqpq$", "ab"},
        {"pqrpqpqabab$", "ab"},
        {"ATCGATCGA$", "ATCGA"},
        {"mississippi$", "issi"},
        {"abcabcaacb$", "abca"},
        {"aababa$", "aba"},
    };
    for (auto& [str, expected] : tests) {
        auto range = step::longest_repeated_substring::find<std::map>(str);
        CHECK(expected == std::string(range.first, range.second));
    }
}

TEST_CASE("longest_repeated_substring_case_insensitive")
{
    auto range =
        step::longest_repeated_substring::find<case_insensitive_unordered_map,
                                               case_insensitive_equal_to>(
            "geeksForGeeks");
    CHECK("geeks" == std::string(range.first, range.second));
}

#endif  // STEP_TEST_LONGEST_REPEATED_SUBSTRING_HPP
