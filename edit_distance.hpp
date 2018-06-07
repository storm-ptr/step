// Andrew Naplavkov

#ifndef STEP_EDIT_DISTANCE_HPP
#define STEP_EDIT_DISTANCE_HPP

#include <algorithm>
#include <step/detail/common.hpp>
#include <step/detail/hirschberg.hpp>

namespace step {
namespace edit_distance {
namespace detail {

template <typename Equal>
struct dynamic_programming {
    Equal equal;

    /// @see https://en.wikipedia.org/wiki/Wagner–Fischer_algorithm
    template <typename RandomIt1, typename RandomIt2>
    auto make_last_row(RandomIt1 first1,
                       RandomIt1 last1,
                       RandomIt2 first2,
                       RandomIt2 last2) const
    {
        size_t size1 = std::distance(first1, last1);
        size_t size2 = std::distance(first2, last2);
        ring_table<size_t, 2> tbl(size2 + 1);
        for (size_t l = 0; l <= size1; ++l)
            for (size_t r = 0; r <= size2; ++r) {
                if (l == 0)
                    tbl[l][r] = r;
                else if (r == 0)
                    tbl[l][r] = l;
                else if (equal(first1[l - 1], first2[r - 1]))
                    tbl[l][r] = tbl[l - 1][r - 1];
                else
                    tbl[l][r] = 1 + std::min({tbl[l][r - 1],        // insert
                                              tbl[l - 1][r],        // remove
                                              tbl[l - 1][r - 1]});  // replace
            }
        return std::move(tbl[size1]);
    }

    bool operator()(size_t lhs, size_t rhs) const { return lhs < rhs; }

    template <typename RandomIt1, typename RandomIt2, typename OutputIt>
    OutputIt trivial_trace(RandomIt1 first1,
                           RandomIt1 last1,
                           RandomIt2 first2,
                           RandomIt2 last2,
                           OutputIt result) const
    {
        if (first1 == last1)
            return std::transform(first2, last2, result, [&](const auto& item) {
                return std::make_pair(std::nullopt, item);
            });
        else if (first2 == last2)
            return std::transform(first1, last1, result, [&](const auto& item) {
                return std::make_pair(item, std::nullopt);
            });
        else if (std::next(first1) == last1)
            return associate_with_equal_or_tail(
                first2, last2, *first1, result, equal, make_reverse_pair{});
        else  // std::next(first2) == last2
            return associate_with_equal_or_tail(
                first1, last1, *first2, result, equal, make_pair{});
    }
};

}  // namespace detail

/**
 * Find the optimal sequence alignment between two strings. Optimality is
 * measured with the Levenshtein distance, defined to be the sum of the costs of
 * insertions, replacements, deletions, and null actions needed to change one
 * string into the other.
 *
 * Time complexity: O(N*M), space complexity O(min(N,M)), where:
 * N = std::distance(first1, last1), M = std::distance(first2, last2).
 */
template <typename RandomIt1,
          typename RandomIt2,
          typename OutputIt,
          typename Equal>
OutputIt associate(RandomIt1 first1,
                   RandomIt1 last1,
                   RandomIt2 first2,
                   RandomIt2 last2,
                   OutputIt result,
                   Equal equal)
{
    return hirschberg::trace(first1,
                             last1,
                             first2,
                             last2,
                             result,
                             detail::dynamic_programming<Equal>{equal});
}

template <typename RandomIt1, typename RandomIt2, typename OutputIt>
OutputIt associate(RandomIt1 first1,
                   RandomIt1 last1,
                   RandomIt2 first2,
                   RandomIt2 last2,
                   OutputIt result)
{
    return edit_distance::associate(
        first1, last1, first2, last2, result, std::equal_to{});
}

template <typename RandomRng1,
          typename RandomRng2,
          typename OutputIt,
          typename Equal>
OutputIt associate(const RandomRng1& rng1,
                   const RandomRng2& rng2,
                   OutputIt result,
                   Equal equal)
{
    return edit_distance::associate(std::begin(rng1),
                                    std::end(rng1),
                                    std::begin(rng2),
                                    std::end(rng2),
                                    result,
                                    equal);
}

template <typename RandomRng1, typename RandomRng2, typename OutputIt>
OutputIt associate(const RandomRng1& rng1,
                   const RandomRng2& rng2,
                   OutputIt result)
{
    return edit_distance::associate(std::begin(rng1),
                                    std::end(rng1),
                                    std::begin(rng2),
                                    std::end(rng2),
                                    result);
}

}  // namespace edit_distance
}  // namespace step

#endif  // STEP_EDIT_DISTANCE_HPP
