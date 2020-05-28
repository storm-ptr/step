// Andrew Naplavkov

#ifndef STEP_LONGEST_REPEATED_SUBSTRING_HPP
#define STEP_LONGEST_REPEATED_SUBSTRING_HPP

#include "suffix_array.hpp"
#include "suffix_tree.hpp"

namespace step::longest_repeated_substring {
namespace detail {

template <class Compare>
struct suffix_array_searcher {
    template <class Size, class RandomIt>
    auto find_with(std::pair<RandomIt, RandomIt> rng) const
    {
        using value_t = iter_value_t<RandomIt>;
        auto result = std::make_pair(rng.second, rng.second);
        auto arr = suffix_array<value_t, Size, Compare>{rng.first, rng.second};
        auto lcp = std::vector<Size>(arr.size());
        arr.longest_common_prefix_array(lcp.begin());
        auto it = std::max_element(lcp.begin(), lcp.end());
        if (it != lcp.end() && *it > 0) {
            auto pos = arr.nth_element((Size)std::distance(lcp.begin(), it));
            result.first = rng.first + pos;
            result.second = result.first + *it;
        }
        return result;
    }
};

template <template <class...> class Map>
struct suffix_tree_searcher {
    template <class Size, class RandomIt>
    auto find_with(std::pair<RandomIt, RandomIt> rng) const
    {
        auto result = std::make_pair(rng.second, rng.second);
        auto tree = suffix_tree<iter_value_t<RandomIt>, Size, Map>{};
        append(tree, rng);
        tree.visit([&](auto& edge) {
            if (edge.visited && edge.path > (Size)size(result)) {
                auto [first, last] = tree.path(edge);
                result.first = rng.first + first;
                result.second = rng.first + last;
            }
        });
        return result;
    }
};

}  // namespace detail

/// Find the longest substring that occurs at least twice in the text.

/// Time complexity O(N*log(N)*log(N)), space complexity O(N), where:
/// N = std::distance(first, last).
/// A suffix array with optional parameter is used under the hood:
/// @tparam Compare - to determine the order of characters.
/// @return a pair of iterators defining the wanted substring.
/// @see https://en.wikipedia.org/wiki/Longest_repeated_substring_problem
template <class Compare = std::less<>, class RandomIt>
auto find_with_suffix_array(RandomIt first, RandomIt last)
{
    auto searcher = detail::suffix_array_searcher<Compare>{};
    return find(searcher, std::make_pair(first, last));
}

template <class Compare = std::less<>, class RandomRng>
auto find_with_suffix_array(const RandomRng& rng)
{
    return longest_repeated_substring::find_with_suffix_array<Compare>(
        std::begin(rng), std::end(rng));
}

/// Find the longest substring that occurs at least twice in the text.

/// The text is padded with unique terminator.
/// Time complexity O(N*log(N)), space complexity O(N), where:
/// N = std::distance(first, last).
/// A suffix tree with optional parameter is used under the hood:
/// @tparam Map - to associate characters with edges.
/// @return a pair of iterators defining the wanted substring.
template <template <class...> class Map = std::unordered_map, class RandomIt>
auto find_with_suffix_tree(RandomIt first, RandomIt last)
{
    auto searcher = detail::suffix_tree_searcher<Map>{};
    return find(searcher, std::make_pair(first, last));
}

template <template <class...> class Map = std::unordered_map, class RandomRng>
auto find_with_suffix_tree(const RandomRng& rng)
{
    return longest_repeated_substring::find_with_suffix_tree<Map>(
        std::begin(rng), std::end(rng));
}

}  // namespace step::longest_repeated_substring

#endif  // STEP_LONGEST_REPEATED_SUBSTRING_HPP
