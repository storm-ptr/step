// Andrew Naplavkov

#ifndef STEP_TEST_LONGEST_INCREASING_SUBSEQUENCE_HPP
#define STEP_TEST_LONGEST_INCREASING_SUBSEQUENCE_HPP

#include <deque>
#include <sstream>
#include <step/longest_increasing_subsequence.hpp>
#include <string>
#include <string_view>

TEST_CASE("longest_increasing_subsequence")
{
    using namespace step::longest_increasing_subsequence;

    std::deque d{0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15};
    std::ostringstream os;
    std::copy(d.begin(), partition(d), std::ostream_iterator<int>(os, " "));
    CHECK(os.str() == "0 2 6 9 11 15 ");

    std::string s{"CBFDEA"};
    CHECK(std::string_view(s.c_str(), std::distance(s.begin(), partition(s))) ==
          "BDE");

    int a[] = {60, 41, 50, 21, 33, 9, 22, 10};
    CHECK(std::vector<int>(std::begin(a), partition(a, std::greater{})) ==
          std::vector{60, 50, 33, 22, 10});

    std::vector perm{1, 1, 2, 2, 3, 3, 4, 4};
    do {
        std::vector lis(perm);
        auto end_of_lis = partition(lis);
        CHECK(std::is_permutation(lis.begin(), lis.end(), perm.begin()));
        CHECK(std::is_sorted(lis.begin(), end_of_lis));
        CHECK(std::distance(lis.begin(), end_of_lis) >= 2);
    } while (std::next_permutation(perm.begin(), perm.end()));
}

#endif  // STEP_TEST_LONGEST_INCREASING_SUBSEQUENCE_HPP
