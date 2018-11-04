// Andrew Naplavkov

#ifndef STEP_LONGEST_COMMON_SUBSTRING_HPP
#define STEP_LONGEST_COMMON_SUBSTRING_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <step/detail/utility.hpp>
#include <step/suffix_array.hpp>
#include <step/suffix_tree.hpp>

/// @see https://en.wikipedia.org/wiki/Longest_common_substring_problem

namespace step {
namespace longest_common_substring {
namespace detail {

// #include <boost/range/join.hpp>
template <class RandomIt1, class RandomIt2>
auto join(RandomIt1 first1, RandomIt1 last1, RandomIt2 first2, RandomIt2 last2)
{
    std::vector<iterator_value<RandomIt1>> result(first1, last1);
    result.insert(result.end(), first2, last2);
    return result;
}

template <class Size, class Compare, class RandomIt1, class RandomIt2>
auto find_with_suffix_array(RandomIt1 first1,
                            RandomIt1 last1,
                            RandomIt2 first2,
                            RandomIt2 last2)
{
    auto result = std::make_pair(last1, last1);
    auto arr = step::suffix_array<iterator_value<RandomIt1>, Size, Compare>{
        join(first1, last1, first2, last2)};
    auto lcp = std::vector<Size>(arr.size());
    arr.longest_common_prefix_array(lcp.begin());
    Size size1 = std::distance(first1, last1);
    for (Size i = 1; i < arr.size(); ++i) {
        if ((arr.nth_element(i - 1) < size1) == (arr.nth_element(i) < size1))
            continue;
        Size pos = std::min<Size>(arr.nth_element(i - 1), arr.nth_element(i));
        Size len = std::min<Size>(lcp[i - 1], size1 - pos);
        if (len > (Size)std::distance(result.first, result.second)) {
            result.first = first1 + pos;
            result.second = result.first + len;
        }
    }
    return result;
}

template <class Size,
          template <class...> class Map,
          class RandomIt1,
          class RandomIt2>
auto find_with_suffix_tree(RandomIt1 first1,
                           RandomIt1 last1,
                           RandomIt2 first2,
                           RandomIt2 last2)
{
    static constexpr uint8_t left_flag = 1;
    static constexpr uint8_t right_flag = 2;

    auto result = std::make_pair(last1, last1);
    auto tree = step::suffix_tree<iterator_value<RandomIt1>, Size, Map>{};
    Size size1 = std::distance(first1, last1);
    Size size2 = std::distance(first2, last2);
    tree.reserve(size1 + size2);
    std::copy(first1, last1, std::back_inserter(tree));
    std::copy(first2, last2, std::back_inserter(tree));
    std::unordered_map<Size, uint8_t> flags;
    tree.visit([](auto&&...) {},
               [&](const auto& str, const auto& parent_str, auto len) {
                   if (!tree.suffix(str))
                       flags[parent_str.first] |= flags[str.first];
                   else if ((str.second - len) < size1)
                       flags[parent_str.first] |= left_flag;
                   else
                       flags[parent_str.first] |= right_flag;
               });
    tree.visit(
        [&](const auto& str, const auto&, auto len) {
            if (!tree.suffix(str) &&
                flags[str.first] == (left_flag | right_flag) &&
                len > (Size)std::distance(result.first, result.second)) {
                result.second = first1 + str.second;
                result.first = result.second - len;
            }
        },
        [](auto&&...) {});
    return result;
}

}  // namespace detail

/**
 * Find the longest string that is a substring of two strings.
 * Time complexity O((N+M)*log(N+M)*log(N+M)), space complexity O(N+M), where:
 * N = std::distance(first1, last1) and M = std::distance(first2, last2).
 * A suffix array with optional parameter is used under the hood:
 * @param Compare - to determine the order of characters.
 * @return a pair of iterators defining the wanted substring.
 */
template <class Compare = std::less<>, class RandomIt1, class RandomIt2>
std::pair<RandomIt1, RandomIt1> find_with_suffix_array(RandomIt1 first1,
                                                       RandomIt1 last1,
                                                       RandomIt2 first2,
                                                       RandomIt2 last2)
{
    size_t size = std::distance(first1, last1) + std::distance(first2, last2);
    if (size < std::numeric_limits<int16_t>::max())
        return detail::find_with_suffix_array<uint16_t, Compare>(
            first1, last1, first2, last2);
    else if (size < std::numeric_limits<int32_t>::max())
        return detail::find_with_suffix_array<uint32_t, Compare>(
            first1, last1, first2, last2);
    else
        return detail::find_with_suffix_array<size_t, Compare>(
            first1, last1, first2, last2);
}

template <class Compare = std::less<>, class RandomRng1, class RandomRng2>
auto find_with_suffix_array(const RandomRng1& rng1, const RandomRng2& rng2)
{
    return longest_common_substring::find_with_suffix_array<Compare>(
        std::begin(rng1), std::end(rng1), std::begin(rng2), std::end(rng2));
}

/**
 * Find the longest string that is a substring of two strings,
 * padded with unique string terminators.
 * Time complexity O((N+M)*log(N+M)), space complexity O(N+M), where:
 * N = std::distance(first1, last1) and M = std::distance(first2, last2).
 * A suffix tree with optional parameter is used under the hood:
 * @param Map - to associate characters with edges.
 * @return a pair of iterators defining the wanted substring.
 */
template <template <class...> class Map = std::unordered_map,
          class RandomIt1,
          class RandomIt2>
std::pair<RandomIt1, RandomIt1> find_with_suffix_tree(RandomIt1 first1,
                                                      RandomIt1 last1,
                                                      RandomIt2 first2,
                                                      RandomIt2 last2)
{
    size_t size = std::distance(first1, last1) + std::distance(first2, last2);
    if (size < std::numeric_limits<int16_t>::max())
        return detail::find_with_suffix_tree<uint16_t, Map>(
            first1, last1, first2, last2);
    else if (size < std::numeric_limits<int32_t>::max())
        return detail::find_with_suffix_tree<uint32_t, Map>(
            first1, last1, first2, last2);
    else
        return detail::find_with_suffix_tree<size_t, Map>(
            first1, last1, first2, last2);
}

template <template <class...> class Map = std::unordered_map,
          class RandomRng1,
          class RandomRng2>
auto find_with_suffix_tree(const RandomRng1& rng1, const RandomRng2& rng2)
{
    return longest_common_substring::find_with_suffix_tree<Map>(
        std::begin(rng1), std::end(rng1), std::begin(rng2), std::end(rng2));
}

}  // namespace longest_common_substring
}  // namespace step

#endif  // STEP_LONGEST_COMMON_SUBSTRING_HPP
