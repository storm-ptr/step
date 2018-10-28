// Andrew Naplavkov

#ifndef STEP_LONGEST_INCREASING_SUBSEQUENCE_HPP
#define STEP_LONGEST_INCREASING_SUBSEQUENCE_HPP

#include <algorithm>
#include <functional>
#include <iterator>
#include <optional>
#include <unordered_map>
#include <vector>

namespace step {
namespace longest_increasing_subsequence {
namespace detail {

/*
 * @see
 * https://www.geeksforgeeks.org/longest-monotonically-increasing-subsequence-size-n-log-n/
 */
class increasing_subsequences {
    std::vector<size_t> tails_;
    std::unordered_map<size_t, std::optional<size_t>> prevs_;

public:
    template <class RandomIt, class Compare>
    increasing_subsequences(RandomIt first, RandomIt last, Compare cmp)
    {
        size_t size = std::distance(first, last);
        for (size_t i = 0; i < size; ++i) {
            auto tail = std::upper_bound(
                tails_.begin(), tails_.end(), i, [&](size_t lhs, size_t rhs) {
                    return cmp(first[lhs], first[rhs]);
                });

            if (tail == tails_.begin())
                prevs_[i] = std::nullopt;
            else
                prevs_[i] = *std::prev(tail);

            if (tail == tails_.end())
                tails_.push_back(i);
            else
                *tail = i;
        }
    }

    auto longest() const
    {
        std::vector<size_t> result;
        auto i = tails_.empty() ? std::nullopt : std::optional{tails_.back()};
        for (; i; i = prevs_.at(i.value()))
            result.push_back(i.value());
        std::reverse(result.begin(), result.end());
        return result;
    }
};

}  // namespace detail

/**
 * Finds a subsequence of a given sequence in which the subsequence's elements
 * are in sorted order, lowest to highest, and in which the subsequence is as
 * long as possible. This subsequence is not necessarily contiguous, or unique.
 * Reorders the elements in such a way that all elements for the subsequence
 * precede the others.
 * @return iterator to the end of the subsequence.
 * Time complexity O(N*log(N)), space complexity O(N),
 * where N = std::distance(first, last).
 * @see https://en.wikipedia.org/wiki/Longest_increasing_subsequence
 */
template <class RandomIt, class Compare>
auto partition(RandomIt first, RandomIt last, Compare cmp)
{
    using std::swap;
    auto it = first;
    for (auto i : detail::increasing_subsequences{first, last, cmp}.longest()) {
        swap(first[i], *it);
        ++it;
    }
    return it;
}

template <class RandomIt>
auto partition(RandomIt first, RandomIt last)
{
    return longest_increasing_subsequence::partition(first, last, std::less{});
}

template <class RandomRng, class Compare>
auto partition(RandomRng& rng, Compare cmp)
{
    return longest_increasing_subsequence::partition(
        std::begin(rng), std::end(rng), cmp);
}

template <class RandomRng>
auto partition(RandomRng& rng)
{
    return longest_increasing_subsequence::partition(std::begin(rng),
                                                     std::end(rng));
}

}  // namespace longest_increasing_subsequence
}  // namespace step

#endif  // STEP_LONGEST_INCREASING_SUBSEQUENCE_HPP
