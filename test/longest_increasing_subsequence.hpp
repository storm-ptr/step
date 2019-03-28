// Andrew Naplavkov

#ifndef STEP_TEST_LONGEST_INCREASING_SUBSEQUENCE_HPP
#define STEP_TEST_LONGEST_INCREASING_SUBSEQUENCE_HPP

#include <deque>
#include <sstream>
#include <step/longest_increasing_subsequence.hpp>
#include <step/test/case_insensitive.hpp>
#include <string_view>

TEST_CASE("longest_increasing_subsequence_hello_world")
{
    std::vector v{6, 3, 4, 8, 10, 5, 7, 1, 9, 2};
    int expect[] = {3, 4, 5, 7, 9};
    auto it = step::longest_increasing_subsequence::partition(v);
    CHECK(std::equal(v.begin(), it, std::begin(expect), std::end(expect)));
}

TEST_CASE("longest_increasing_subsequence_partition")
{
    using namespace step::longest_increasing_subsequence;

    std::deque dq{0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15};
    std::ostringstream os;
    std::copy(dq.begin(), partition(dq), std::ostream_iterator<int>(os, " "));
    CHECK(os.str() == "0 2 6 9 11 15 ");

    std::string str{"CBfdEA"};
    auto it = partition(str, step::case_insensitive::less{});
    CHECK(std::string_view(str.c_str(), std::distance(str.begin(), it)) ==
          "BdE");

    int arr[] = {60, 41, 50, 21, 33, 9, 22, 10};
    CHECK(std::vector<int>(std::begin(arr), partition(arr, std::greater{})) ==
          std::vector{60, 50, 33, 22, 10});

    std::vector perm{1, 1, 2, 2, 3, 3, 4, 4};
    do {
        std::vector lis(perm);
        auto last = partition(lis);
        CHECK(std::is_permutation(lis.begin(), lis.end(), perm.begin()));
        CHECK(std::is_sorted(lis.begin(), last));
        CHECK(std::distance(lis.begin(), last) >= 2);
    } while (std::next_permutation(perm.begin(), perm.end()));
}

#endif  // STEP_TEST_LONGEST_INCREASING_SUBSEQUENCE_HPP
