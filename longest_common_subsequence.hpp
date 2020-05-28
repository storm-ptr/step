// Andrew Naplavkov

#ifndef STEP_LONGEST_COMMON_SUBSEQUENCE_HPP
#define STEP_LONGEST_COMMON_SUBSEQUENCE_HPP

#include "detail/hirschberg.hpp"

namespace step::longest_common_subsequence {
namespace detail {

template <class Equal>
struct dynamic_programming {
    Equal eq;

    template <class RandomIt1, class RandomIt2>
    auto make_last_row(RandomIt1 first1,
                       RandomIt1 last1,
                       RandomIt2 first2,
                       RandomIt2 last2) const
    {
        size_t size1 = std::distance(first1, last1);
        size_t size2 = std::distance(first2, last2);
        ring_table<size_t, 2> tbl(size2 + 1);
        for (size_t l = 1; l <= size1; ++l)
            for (size_t r = 1; r <= size2; ++r)
                tbl[l][r] = eq(first1[l - 1], first2[r - 1])
                                ? tbl[l - 1][r - 1] + 1
                                : std::max(tbl[l - 1][r], tbl[l][r - 1]);
        return std::move(tbl[size1]);
    }

    bool operator()(size_t lhs, size_t rhs) const { return lhs > rhs; }

    template <class RandomIt1, class RandomIt2, class OutputIt>
    OutputIt trivial_trace(RandomIt1 first1,
                           RandomIt1 last1,
                           RandomIt2 first2,
                           RandomIt2 last2,
                           OutputIt result) const
    {
        auto it = std::find_first_of(first1, last1, first2, last2, eq);
        if (it != last1)
            *result++ = *it;
        return result;
    }
};

}  // namespace detail

/// Find the longest subsequence present in two sequences.

/// A subsequence is a sequence that appears in the same relative order,
/// but not necessarily contiguous.
/// Time complexity O(N*M), space complexity O(min(N,M)), where:
/// N = std::distance(first1, last1), M = std::distance(first2, last2).
/// @see https://en.wikipedia.org/wiki/Longest_common_subsequence_problem
/// @see https://www.geeksforgeeks.org/longest-common-subsequence/
template <class RandomIt1, class RandomIt2, class OutputIt, class Equal>
OutputIt intersection(RandomIt1 first1,
                      RandomIt1 last1,
                      RandomIt2 first2,
                      RandomIt2 last2,
                      OutputIt result,
                      Equal&& eq)
{
    return hirschberg::trace(
        first1,
        last1,
        first2,
        last2,
        result,
        detail::dynamic_programming<Equal>{std::forward<Equal>(eq)});
}

template <class RandomIt1, class RandomIt2, class OutputIt>
OutputIt intersection(RandomIt1 first1,
                      RandomIt1 last1,
                      RandomIt2 first2,
                      RandomIt2 last2,
                      OutputIt result)
{
    return longest_common_subsequence::intersection(
        first1, last1, first2, last2, result, std::equal_to{});
}

template <class RandomRng1, class RandomRng2, class OutputIt, class Equal>
OutputIt intersection(const RandomRng1& rng1,
                      const RandomRng2& rng2,
                      OutputIt result,
                      Equal&& eq)
{
    return longest_common_subsequence::intersection(std::begin(rng1),
                                                    std::end(rng1),
                                                    std::begin(rng2),
                                                    std::end(rng2),
                                                    result,
                                                    std::forward<Equal>(eq));
}

template <class RandomRng1, class RandomRng2, class OutputIt>
OutputIt intersection(const RandomRng1& rng1,
                      const RandomRng2& rng2,
                      OutputIt result)
{
    return longest_common_subsequence::intersection(std::begin(rng1),
                                                    std::end(rng1),
                                                    std::begin(rng2),
                                                    std::end(rng2),
                                                    result);
}

}  // namespace step::longest_common_subsequence

#endif  // STEP_LONGEST_COMMON_SUBSEQUENCE_HPP
