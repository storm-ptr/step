// Andrew Naplavkov

#ifndef STEP_EDIT_DISTANCE_HPP
#define STEP_EDIT_DISTANCE_HPP

#include "detail/hirschberg.hpp"
#include <optional>

namespace step::edit_distance {
namespace detail {

template <class ForwardIt, class T, class OutputIt, class Equal, class BinaryOp>
auto join_on_equal_or_tail(ForwardIt first,
                           ForwardIt last,
                           const T& value,
                           OutputIt result,
                           Equal eq,
                           BinaryOp op)
{
    bool done = false;
    while (first != last) {
        auto next = std::next(first);
        if (!done && (eq(*first, value) || next == last)) {
            done = true;
            *result++ = op(*first, value);
        }
        else
            *result++ = op(*first, std::nullopt);
        first = next;
    }
    return result;
}

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
        for (size_t l = 0; l <= size1; ++l)
            for (size_t r = 0; r <= size2; ++r) {
                if (l == 0)
                    tbl[l][r] = r;
                else if (r == 0)
                    tbl[l][r] = l;
                else if (eq(first1[l - 1], first2[r - 1]))
                    tbl[l][r] = tbl[l - 1][r - 1];
                else
                    tbl[l][r] = 1 + std::min({tbl[l][r - 1],        // insert
                                              tbl[l - 1][r],        // remove
                                              tbl[l - 1][r - 1]});  // replace
            }
        return std::move(tbl[size1]);
    }

    bool operator()(size_t lhs, size_t rhs) const { return lhs < rhs; }

    template <class RandomIt1, class RandomIt2, class OutputIt>
    OutputIt trivial_trace(RandomIt1 first1,
                           RandomIt1 last1,
                           RandomIt2 first2,
                           RandomIt2 last2,
                           OutputIt result) const
    {
        if (first1 == last1)
            return std::transform(first2, last2, result, [&](auto& item) {
                return std::make_pair(std::nullopt, item);
            });
        else if (first2 == last2)
            return std::transform(first1, last1, result, [&](auto& item) {
                return std::make_pair(item, std::nullopt);
            });
        else if (std::next(first1) == last1)
            return join_on_equal_or_tail(
                first2, last2, *first1, result, eq, make_reverse_pair{});
        else  // std::next(first2) == last2
            return join_on_equal_or_tail(
                first1, last1, *first2, result, eq, make_pair{});
    }
};

}  // namespace detail

/// Find the optimal sequence alignment between two strings.

/// Optimality is measured with the Levenshtein distance,
/// defined to be the sum of the costs of
/// insertions, replacements, deletions, and null actions needed
/// to change one string into the other.
/// Time complexity O(N*M), space complexity O(min(N,M)), where:
/// N = std::distance(first1, last1), M = std::distance(first2, last2).
/// @see https://en.wikipedia.org/wiki/Levenshtein_distance
/// @see https://en.wikipedia.org/wiki/Wagner%E2%80%93Fischer_algorithm
template <class RandomIt1, class RandomIt2, class OutputIt, class Equal>
OutputIt join(RandomIt1 first1,
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
OutputIt join(RandomIt1 first1,
              RandomIt1 last1,
              RandomIt2 first2,
              RandomIt2 last2,
              OutputIt result)
{
    return edit_distance::join(
        first1, last1, first2, last2, result, std::equal_to{});
}

template <class RandomRng1, class RandomRng2, class OutputIt, class Equal>
OutputIt join(const RandomRng1& rng1,
              const RandomRng2& rng2,
              OutputIt result,
              Equal&& eq)
{
    return edit_distance::join(std::begin(rng1),
                               std::end(rng1),
                               std::begin(rng2),
                               std::end(rng2),
                               result,
                               std::forward<Equal>(eq));
}

template <class RandomRng1, class RandomRng2, class OutputIt>
OutputIt join(const RandomRng1& rng1, const RandomRng2& rng2, OutputIt result)
{
    return edit_distance::join(std::begin(rng1),
                               std::end(rng1),
                               std::begin(rng2),
                               std::end(rng2),
                               result);
}

}  // namespace step::edit_distance

#endif  // STEP_EDIT_DISTANCE_HPP
