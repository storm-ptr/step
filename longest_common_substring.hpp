// Andrew Naplavkov

#ifndef STEP_LONGEST_COMMON_SUBSTRING_HPP
#define STEP_LONGEST_COMMON_SUBSTRING_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <step/suffix_tree.hpp>
#include <unordered_map>

namespace step {
namespace longest_common_substring {
namespace detail {

template <class Size,
          template <class...> class Map,
          class Equal,
          class RandomIt1,
          class RandomIt2>
auto find(RandomIt1 first1, RandomIt1 last1, RandomIt2 first2, RandomIt2 last2)
{
    using value_type = typename std::iterator_traits<RandomIt1>::value_type;
    static constexpr auto left_flag = (std::byte)1;
    static constexpr auto right_flag = (std::byte)2;

    Size size1 = std::distance(first1, last1);
    Size size2 = std::distance(first2, last2);
    step::suffix_tree<value_type, Size, Map, Equal> tree{};
    tree.reserve(size1 + size2);
    std::copy(first1, last1, std::back_inserter(tree));
    std::copy(first2, last2, std::back_inserter(tree));

    std::unordered_map<Size, std::byte> flags;
    tree.visit([](auto&&...) {},
               [&](const auto& str, const auto& parent_str, auto len) {
                   if (!tree.suffix(str))
                       flags[parent_str.first] |= flags[str.first];
                   else if ((str.second - len) < size1)
                       flags[parent_str.first] |= left_flag;
                   else
                       flags[parent_str.first] |= right_flag;
               });

    std::pair<RandomIt1, RandomIt1> result{last1, last1};
    tree.visit(
        [&](const auto& str, const auto&, auto len) {
            if (!tree.suffix(str) &&
                flags[str.first] == (left_flag | right_flag) &&
                len > (Size)std::distance(result.first, result.second))
                result = std::make_pair(first1 + str.second - len,
                                        first1 + str.second);
        },
        [](auto&&...) {});
    return result;
}

}  // namespace detail

/**
 * Find the longest string that is a substring of two strings,
 * padded with unique string terminators.
 *
 * Time complexity O((N+M)*log(N+M)), space complexity O(N+M), where:
 * N = std::distance(first1, last1) and M = std::distance(first2, last2).
 *
 * A suffix tree with optional parameters is used under the hood:
 * @param Map - an associative container that is used to to store the edges;
 * @param Equal - to determine whether two characters are equivalent.
 *
 * @return a pair of iterators defining the wanted substring.
 *
 * @see https://en.wikipedia.org/wiki/Longest_common_substring_problem
 */
template <template <class...> class Map = std::unordered_map,
          class Equal = std::equal_to<>,
          class RandomIt1,
          class RandomIt2>
std::pair<RandomIt1, RandomIt1> find(RandomIt1 first1,
                                     RandomIt1 last1,
                                     RandomIt2 first2,
                                     RandomIt2 last2)
{
    size_t size = std::distance(first1, last1) + std::distance(first2, last2);
    if (size < std::numeric_limits<int16_t>::max())
        return detail::find<uint16_t, Map, Equal>(first1, last1, first2, last2);
    else if (size < std::numeric_limits<int32_t>::max())
        return detail::find<uint32_t, Map, Equal>(first1, last1, first2, last2);
    else
        return detail::find<size_t, Map, Equal>(first1, last1, first2, last2);
}

template <template <class...> class Map = std::unordered_map,
          class Equal = std::equal_to<>,
          class RandomRng1,
          class RandomRng2>
auto find(const RandomRng1& rng1, const RandomRng2& rng2)
{
    return longest_common_substring::find<Map, Equal>(
        std::begin(rng1), std::end(rng1), std::begin(rng2), std::end(rng2));
}

}  // namespace longest_common_substring
}  // namespace step

#endif  // STEP_LONGEST_COMMON_SUBSTRING_HPP
