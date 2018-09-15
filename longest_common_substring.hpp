// Andrew Naplavkov

#ifndef STEP_LONGEST_COMMON_SUBSTRING_HPP
#define STEP_LONGEST_COMMON_SUBSTRING_HPP

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <step/suffix_tree.hpp>
#include <unordered_map>

namespace step {
namespace longest_common_substring {
namespace detail {

template <template <class...> class Map,
          class Equal,
          class Size,
          class RandomIt1,
          class RandomIt2>
auto find(RandomIt1 first1,
          RandomIt1 last1,
          RandomIt2 first2,
          RandomIt2 last2,
          Equal equal)
{
    using value_type = typename std::iterator_traits<RandomIt1>::value_type;
    static constexpr uint8_t left_flag = 1;
    static constexpr uint8_t right_flag = 2;

    step::suffix_tree<value_type, Map, Equal, Size> tree{equal};
    tree.reserve((last1 - first1) + (last2 - first2));
    std::copy(first1, last1, std::back_inserter(tree));
    std::copy(first2, last2, std::back_inserter(tree));

    std::unordered_map<Size, uint8_t> flags;
    tree.visit([](auto&&...) {},
               [&](const auto& str, const auto& parent_str, auto len) {
                   if (!tree.suffix(str))
                       flags[parent_str.first] |= flags[str.first];
                   else if ((str.second - len) < (last1 - first1))
                       flags[parent_str.first] |= left_flag;
                   else
                       flags[parent_str.first] |= right_flag;
               });

    std::pair<RandomIt1, RandomIt1> result{last1, last1};
    tree.visit(
        [&](const auto& str, const auto&, auto len) {
            if (!tree.suffix(str) &&
                flags[str.first] == (left_flag | right_flag) &&
                len > Size(result.second - result.first))
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
 * Time complexity O(N*log(N)), space complexity O(N), where:
 * N = std::distance(first1, last1) + std::distance(first2, last2).
 *
 * A suffix tree with optional parameters is used under the hood:
 * @param Map - an associative container that is used to to store the edges;
 * @param Equal - to determine whether two characters are equivalent.
 *
 * @see https://en.wikipedia.org/wiki/Longest_common_substring_problem
 */
template <template <class...> class Map = std::unordered_map,
          class Equal,
          class RandomIt1,
          class RandomIt2>
auto find(RandomIt1 first1,
          RandomIt1 last1,
          RandomIt2 first2,
          RandomIt2 last2,
          Equal equal)
{
    if ((last1 - first1) + (last2 - first2) <
        std::numeric_limits<int32_t>::max())
        return detail::find<Map, Equal, uint32_t>(
            first1, last1, first2, last2, equal);
    else
        return detail::find<Map, Equal, size_t>(
            first1, last1, first2, last2, equal);
}

template <template <class...> class Map = std::unordered_map,
          class RandomIt1,
          class RandomIt2>
auto find(RandomIt1 first1, RandomIt1 last1, RandomIt2 first2, RandomIt2 last2)
{
    return longest_common_substring::find<Map>(
        first1, last1, first2, last2, std::equal_to{});
}

template <template <class...> class Map = std::unordered_map,
          class Equal,
          class RandomRng1,
          class RandomRng2>
auto find(RandomRng1 rng1, RandomRng2 rng2, Equal equal)
{
    return longest_common_substring::find<Map, Equal>(std::begin(rng1),
                                                      std::end(rng1),
                                                      std::begin(rng2),
                                                      std::end(rng2),
                                                      equal);
}

template <template <class...> class Map = std::unordered_map,
          class RandomRng1,
          class RandomRng2>
auto find(RandomRng1 rng1, RandomRng2 rng2)
{
    return longest_common_substring::find<Map>(
        std::begin(rng1), std::end(rng1), std::begin(rng2), std::end(rng2));
}

}  // namespace longest_common_substring
}  // namespace step

#endif  // STEP_LONGEST_COMMON_SUBSTRING_HPP
