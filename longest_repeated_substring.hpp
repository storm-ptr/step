// Andrew Naplavkov

#ifndef STEP_LONGEST_REPEATED_SUBSTRING_HPP
#define STEP_LONGEST_REPEATED_SUBSTRING_HPP

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <step/suffix_tree.hpp>
#include <unordered_map>

namespace step {
namespace longest_repeated_substring {
namespace detail {

template <template <class...> class Map,
          class Equal,
          class Size,
          class RandomIt>
auto find(RandomIt first, RandomIt last, Equal equal)
{
    using value_type = typename std::iterator_traits<RandomIt>::value_type;
    step::suffix_tree<value_type, Map, Equal, Size> tree{equal};
    tree.reserve(last - first);
    std::copy(first, last, std::back_inserter(tree));
    std::pair<RandomIt, RandomIt> result{last, last};
    tree.visit(
        [&](const auto& str, const auto&, auto len) {
            if (!tree.suffix(str) && len > Size(result.second - result.first))
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
 * @param Map - an associative container that is used to to store the edges;
 * @param Equal - to determine whether two characters are equivalent.
 *
 * @see https://en.wikipedia.org/wiki/Longest_repeated_substring_problem
 */
template <template <class...> class Map = std::unordered_map,
          class Equal,
          class RandomIt>
auto find(RandomIt first, RandomIt last, Equal equal)
{
    if ((last - first) < std::numeric_limits<int32_t>::max())
        return detail::find<Map, Equal, uint32_t>(first, last, equal);
    else
        return detail::find<Map, Equal, size_t>(first, last, equal);
}

template <template <class...> class Map = std::unordered_map, class RandomIt>
auto find(RandomIt first, RandomIt last)
{
    return longest_repeated_substring::find<Map>(first, last, std::equal_to{});
}

template <template <class...> class Map = std::unordered_map,
          class Equal,
          class RandomRng>
auto find(const RandomRng& rng, Equal equal)
{
    return longest_repeated_substring::find<Map, Equal>(
        std::begin(rng), std::end(rng), equal);
}

template <template <class...> class Map = std::unordered_map, class RandomRng>
auto find(const RandomRng& rng)
{
    return longest_repeated_substring::find<Map>(std::begin(rng),
                                                 std::end(rng));
}

}  // namespace longest_repeated_substring
}  // namespace step

#endif  // STEP_LONGEST_REPEATED_SUBSTRING_HPP
