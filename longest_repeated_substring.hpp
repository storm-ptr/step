// Andrew Naplavkov

#ifndef STEP_LONGEST_REPEATED_SUBSTRING_HPP
#define STEP_LONGEST_REPEATED_SUBSTRING_HPP

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <step/suffix_tree.hpp>

namespace step {
namespace longest_repeated_substring {
namespace detail {

template <template <class...> class Map, class Len, class RandomIt>
auto find(RandomIt first, RandomIt last)
{
    std::pair<RandomIt, RandomIt> result{last, last};
    using value_type = typename std::iterator_traits<RandomIt>::value_type;
    step::suffix_tree<value_type, Map, Len> tree{};
    tree.reserve(last - first);
    std::copy(first, last, std::back_inserter(tree));
    if (!tree.explicit_())
        throw std::logic_error{"terminal symbol is required"};
    tree.visit([&](auto first_pos, auto, auto last_pos) {
        if (last_pos != tree.size() /* internal */ &&
            last_pos - first_pos > Len(result.second - result.first))
            result = std::make_pair(first + first_pos, first + last_pos);
    });
    return result;
}

}  // namespace detail

/**
 * Find the longest substring of a string that occurs at least twice.
 * A terminal symbol is required.
 *
 * Time complexity O(N*log(N)), space complexity O(N), where:
 * N = std::distance(first, last).
 *
 * @see https://en.wikipedia.org/wiki/Longest_repeated_substring_problem
 */
template <template <class...> class Map = std::map, class RandomIt>
auto find(RandomIt first, RandomIt last)
{
    if ((last - first) < std::numeric_limits<int32_t>::max())
        return detail::find<Map, uint32_t>(first, last);
    else
        return detail::find<Map, size_t>(first, last);
}

template <template <class...> class Map = std::map, class RandomRng>
auto find(const RandomRng& rng)
{
    return longest_repeated_substring::find(std::begin(rng), std::end(rng));
}

}  // namespace longest_repeated_substring
}  // namespace step

#endif  // STEP_LONGEST_REPEATED_SUBSTRING_HPP
