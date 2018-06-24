// Andrew Naplavkov

#ifndef STEP_TEST_LONGEST_COMMON_SUBSEQUENCE_HPP
#define STEP_TEST_LONGEST_COMMON_SUBSEQUENCE_HPP

#include <cctype>
#include <cstring>
#include <sstream>
#include <step/longest_common_subsequence.hpp>
#include <string>
#include <string_view>

TEST_CASE("longest_common_subsequence")
{
    using namespace step::longest_common_subsequence;
    std::ostringstream os;
    intersection(
        std::string{"XMJYAUZ"}, "MZJAWXU", std::ostream_iterator<char>(os));
    CHECK(os.str() == "MJAU");

    std::string s;
    intersection("AGGTAB", std::string_view{"GXTXAYB"}, std::back_inserter(s));
    CHECK(s == "GTAB");

    std::vector<char> v;
    intersection(
        "ABCDGH", "aedfhr", std::back_inserter(v), [](char lhs, char rhs) {
            return tolower(lhs) == tolower(rhs);
        });
    CHECK(strcmp(v.data(), "ADH") == 0);
}

#endif  // STEP_TEST_LONGEST_COMMON_SUBSEQUENCE_HPP
