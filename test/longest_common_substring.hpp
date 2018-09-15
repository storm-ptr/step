// Andrew Naplavkov

#ifndef STEP_TEST_LONGEST_COMMON_SUBSTRING_HPP
#define STEP_TEST_LONGEST_COMMON_SUBSTRING_HPP

#include <map>
#include <step/longest_common_substring.hpp>
#include <string>
#include <string_view>

TEST_CASE("longest_common_substring")
{
    using namespace step::longest_common_substring;
    struct {
        std::string_view lhs;
        std::string_view rhs;
        std::string_view expected;
    } CASES[] = {
        {"xabxac#", "abcabxabcd$", "abxa"},
        {"xabxaabxa#", "babxba$", "abx"},
        {"GeeksforGeeks#", "GeeksQuiz$", "Geeks"},
        {"OldSite:GeeksforGeeks.org#", "NewSite:GeeksQuiz.com$", "Site:Geeks"},
        {"abcde#", "fghie$", "e"},
        {"pqrst#", "uvwxyz$", ""},
    };
    for (auto& [lhs, rhs, expected] : CASES) {
        auto r = step::longest_common_substring::find<std::map>(lhs, rhs);
        CHECK(expected == std::string{r.first, r.second});
    }
}

#endif  // STEP_TEST_LONGEST_COMMON_SUBSTRING_HPP
