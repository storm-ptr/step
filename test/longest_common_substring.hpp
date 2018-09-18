// Andrew Naplavkov

#ifndef STEP_TEST_LONGEST_COMMON_SUBSTRING_HPP
#define STEP_TEST_LONGEST_COMMON_SUBSTRING_HPP

#include <map>
#include <step/longest_common_substring.hpp>
#include <step/test/utility.hpp>
#include <string_view>

TEST_CASE("longest_common_substring_hello_world")
{
    auto range = step::longest_common_substring::find(
        "the longest string that is #", "a substring of two strings $");
    CHECK(" string" == std::string(range.first, range.second));
}

TEST_CASE("longest_common_substring_find")
{
    struct {
        std::string_view lhs;
        std::string_view rhs;
        std::string_view expected;
    } tests[] = {
        {"xabxac#", "abcabxabcd$", "abxa"},
        {"xabxaabxa#", "babxba$", "abx"},
        {"GeeksforGeeks#", "GeeksQuiz$", "Geeks"},
        {"OldSite:GeeksforGeeks.org#", "NewSite:GeeksQuiz.com$", "Site:Geeks"},
        {"abcde#", "fghie$", "e"},
        {"pqrst#", "uvwxyz$", ""},
    };
    for (auto& [lhs, rhs, expected] : tests) {
        auto r = step::longest_common_substring::find<std::map>(lhs, rhs);
        CHECK(expected == std::string{r.first, r.second});
    }
}

TEST_CASE("longest_common_substring_case_insensitive")
{
    auto range =
        step::longest_common_substring::find<case_insensitive_unordered_map,
                                             case_insensitive_equal_to>(
            "geeksforGeeks#", "GEEKSQUIZ$");
    CHECK("geeks" == std::string(range.first, range.second));
}

#endif  // STEP_TEST_LONGEST_COMMON_SUBSTRING_HPP
