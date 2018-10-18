// Andrew Naplavkov

#ifndef STEP_LONGEST_REPEATED_SUBSTRING_HPP
#define STEP_LONGEST_REPEATED_SUBSTRING_HPP

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <step/suffix_tree.hpp>
#include <unordered_map>

namespace step {
namespace longest_repeated_substring {
namespace detail {

template <class Size,
          template <class...> class Map,
          class Equal,
          class RandomIt>
auto find(RandomIt first, RandomIt last)
{
    using value_type = typename std::iterator_traits<RandomIt>::value_type;
    step::suffix_tree<value_type, Size, Map, Equal> tree{};
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
 *
 * @see https://en.wikipedia.org/wiki/Longest_repeated_substring_problem
 */
template <template <class...> class Map = std::unordered_map,
          class Equal = std::equal_to<>,
          class RandomIt>
std::pair<RandomIt, RandomIt> find(RandomIt first, RandomIt last)
{
    size_t size = std::distance(first, last);
    if (size < std::numeric_limits<int16_t>::max())
        return detail::find<uint16_t, Map, Equal>(first, last);
    else if (size < std::numeric_limits<int32_t>::max())
        return detail::find<uint32_t, Map, Equal>(first, last);
    else
        return detail::find<size_t, Map, Equal>(first, last);
}

template <template <class...> class Map = std::unordered_map,
          class Equal = std::equal_to<>,
          class RandomRng>
auto find(const RandomRng& rng)
{
    return longest_repeated_substring::find<Map, Equal>(std::begin(rng),
                                                        std::end(rng));
}

}  // namespace longest_repeated_substring
}  // namespace step

#endif  // STEP_LONGEST_REPEATED_SUBSTRING_HPP
