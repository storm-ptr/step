// Andrew Naplavkov

#ifndef STEP_TEST_LONGEST_COMMON_SUBSEQUENCE_HPP
#define STEP_TEST_LONGEST_COMMON_SUBSEQUENCE_HPP

#include <step/longest_common_subsequence.hpp>
#include <step/test/utility.hpp>
#include <string_view>

TEST_CASE("longest_common_subsequence_hello_world")
{
    using namespace std::literals;
    std::string str;
    step::longest_common_subsequence::intersection("LCS is the basis of "sv,
                                                   "the diff utility"sv,
                                                   std::back_inserter(str));
    CHECK(str == "the if ");
}

TEST_CASE("longest_common_subsequence_intersection")
{
    struct {
        std::string_view lhs;
        std::string_view rhs;
        std::string_view expect;
    } tests[] = {{"XMJYAUZ", "MZJAWXU", "MJAU"},
                 {"AGGTAB", "GXTXAYB", "GTAB"},
                 {"ABCDGH", "aedfhr", "ADH"},
                 {"BANANA", "ATANA", "AANA"},
                 {"gac", "AGCAT", "ga"}};
    for (auto& [lhs, rhs, expect] : tests) {
        std::string str;
        step::longest_common_subsequence::intersection(
            lhs, rhs, std::back_inserter(str), case_insensitive_equal_to{});
        CHECK(str == expect);
    }
}

#endif  // STEP_TEST_LONGEST_COMMON_SUBSEQUENCE_HPP
