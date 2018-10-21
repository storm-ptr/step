// Andrew Naplavkov

#ifndef STEP_LONGEST_REPEATED_SUBSTRING_HPP
#define STEP_LONGEST_REPEATED_SUBSTRING_HPP

#include <algorithm>
#include <cstdint>
#include <functional>
#include <step/detail/utility.hpp>
#include <step/suffix_array.hpp>
#include <step/suffix_tree.hpp>
#include <unordered_map>

/// @see https://en.wikipedia.org/wiki/Longest_repeated_substring_problem

namespace step {
namespace longest_repeated_substring {
namespace detail {

template <class Size, class Compare, class RandomIt>
auto find_with_suffix_array(RandomIt first, RandomIt last)
{
    auto arr = enhanced_suffix_array<iterator_value_t<RandomIt>, Size, Compare>{
        first, last};
    auto& lcp = arr.longest_common_prefix();
    auto it = std::max_element(lcp.begin(), lcp.end());
    std::pair<RandomIt, RandomIt> result{last, last};
    if (it != lcp.end() && *it > 0) {
        auto pos = arr.index()[std::distance(lcp.begin(), it)];
        result.first = first + pos;
        result.second = result.first + *it;
    }
    return result;
}

template <class Size,
          template <class...> class Map,
          class Equal,
          class RandomIt>
auto find_with_suffix_tree(RandomIt first, RandomIt last)
{
    suffix_tree<iterator_value_t<RandomIt>, Size, Map, Equal> tree{};
    tree.reserve(std::distance(first, last));
    std::copy(first, last, std::back_inserter(tree));
    std::pair<RandomIt, RandomIt> result{last, last};
    tree.visit(
        [&](const auto& str, const auto&, auto len) {
            if (!tree.suffix(str) &&
                len > (Size)std::distance(result.first, result.second))
                result = std::make_pair(first + str.second - len,
                                        first + str.second);
        },
        [](auto&&...) {});
    return result;
}

}  // namespace detail

/**
 * Find the longest substring of a string (padded with unique string terminator)
 * that occurs at least twice.
 *
 * Time complexity O(N*log(N)), space complexity O(N), where:
 * N = std::distance(first, last).
 *
 * A suffix tree with optional parameters is used under the hood:
 * @param Map - to associate characters with edges;
 * @param Equal - to determine whether two characters are equivalent.
 *
 * @return a pair of iterators defining the wanted substring.
 */
template <template <class...> class Map = std::unordered_map,
          class Equal = std::equal_to<>,
          class RandomIt>
auto find_with_suffix_tree(RandomIt first, RandomIt last)
{
    size_t size = std::distance(first, last);
    if (size < std::numeric_limits<int16_t>::max())
        return detail::find_with_suffix_tree<uint16_t, Map, Equal>(first, last);
    else if (size < std::numeric_limits<int32_t>::max())
        return detail::find_with_suffix_tree<uint32_t, Map, Equal>(first, last);
    else
        return detail::find_with_suffix_tree<size_t, Map, Equal>(first, last);
}

template <template <class...> class Map = std::unordered_map,
          class Equal = std::equal_to<>,
          class RandomRng>
auto find_with_suffix_tree(const RandomRng& rng)
{
    return longest_repeated_substring::find_with_suffix_tree<Map, Equal>(
        std::begin(rng), std::end(rng));
}

/**
 * Find the longest substring of a string that occurs at least twice.
 *
 * Time complexity O(N*log(N)*log(N)), space complexity O(N), where:
 * N = std::distance(first, last).
 *
 * A suffix array with optional parameter is used under the hood:
 * @param Compare - to determine the order of characters.
 *
 * @return a pair of iterators defining the wanted substring.
 */
template <class Compare = std::less<>, class RandomIt>
auto find_with_suffix_array(RandomIt first, RandomIt last)
{
    size_t size = std::distance(first, last);
    if (size < std::numeric_limits<int16_t>::max())
        return detail::find_with_suffix_array<uint16_t, Compare>(first, last);
    else if (size < std::numeric_limits<int32_t>::max())
        return detail::find_with_suffix_array<uint32_t, Compare>(first, last);
    else
        return detail::find_with_suffix_array<size_t, Compare>(first, last);
}

template <class Compare = std::less<>, class RandomRng>
auto find_with_suffix_array(const RandomRng& rng)
{
    return longest_repeated_substring::find_with_suffix_array<Compare>(
        std::begin(rng), std::end(rng));
}

}  // namespace longest_repeated_substring
}  // namespace step

#endif  // STEP_LONGEST_REPEATED_SUBSTRING_HPP
