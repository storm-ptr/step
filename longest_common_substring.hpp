// Andrew Naplavkov

#ifndef STEP_LONGEST_COMMON_SUBSTRING_HPP
#define STEP_LONGEST_COMMON_SUBSTRING_HPP

#include <algorithm>
#include <step/suffix_array.hpp>
#include <step/suffix_tree.hpp>
#include <step/utility.hpp>

/// @see https://en.wikipedia.org/wiki/Longest_common_substring_problem
namespace step::longest_common_substring {
namespace detail {

template <class Compare>
struct suffix_array_searcher {
    template <class Size, class RandomIt1, class RandomIt2>
    auto operator()(Size,
                    std::pair<RandomIt1, RandomIt1> rng1,
                    std::pair<RandomIt2, RandomIt2> rng2) const
    {
        using value_t = iterator_value<RandomIt1>;
        auto result = std::make_pair(rng1.second, rng1.second);
        auto str = std::vector<value_t>{};
        append(str, rng1, rng2);
        auto arr = suffix_array<value_t, Size, Compare>{std::move(str)};
        auto lcp = std::vector<Size>(arr.size());
        arr.longest_common_prefix_array(lcp.begin());
        Size size1 = std::distance(rng1.first, rng1.second);
        for (Size i = 1; i < arr.size(); ++i) {
            auto prev = arr.nth_element(i - 1);
            auto cur = arr.nth_element(i);
            if ((prev < size1) != (cur < size1)) {
                auto pos = std::min<Size>(prev, cur);
                auto len = std::min<Size>(lcp[i - 1], size1 - pos);
                if (len > (Size)std::distance(result.first, result.second)) {
                    result.first = rng1.first + pos;
                    result.second = result.first + len;
                }
            }
        }
        return result;
    }
};

template <template <class...> class Map>
struct suffix_tree_searcher {
    inline static const uint8_t left_flag = 1;
    inline static const uint8_t right_flag = 2;

    template <class Size, class RandomIt1, class RandomIt2>
    auto operator()(Size,
                    std::pair<RandomIt1, RandomIt1> rng1,
                    std::pair<RandomIt2, RandomIt2> rng2) const
    {
        auto result = std::make_pair(rng1.second, rng1.second);
        auto tree = suffix_tree<iterator_value<RandomIt1>, Size, Map>{};
        append(tree, rng1, rng2);
        auto flags = std::unordered_map<Size, uint8_t>{};
        Size size1 = std::distance(rng1.first, rng1.second);
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
                    result.second = rng1.first + str.second;
                    result.first = result.second - len;
                }
            },
            [](auto&&...) {});
        return result;
    }
};

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
auto find_with_suffix_array(RandomIt1 first1,
                            RandomIt1 last1,
                            RandomIt2 first2,
                            RandomIt2 last2)
{
    auto searcher = detail::suffix_array_searcher<Compare>{};
    return invoke(
        searcher, std::make_pair(first1, last1), std::make_pair(first2, last2));
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
auto find_with_suffix_tree(RandomIt1 first1,
                           RandomIt1 last1,
                           RandomIt2 first2,
                           RandomIt2 last2)
{
    auto searcher = detail::suffix_tree_searcher<Map>{};
    return invoke(
        searcher, std::make_pair(first1, last1), std::make_pair(first2, last2));
}

template <template <class...> class Map = std::unordered_map,
          class RandomRng1,
          class RandomRng2>
auto find_with_suffix_tree(const RandomRng1& rng1, const RandomRng2& rng2)
{
    return longest_common_substring::find_with_suffix_tree<Map>(
        std::begin(rng1), std::end(rng1), std::begin(rng2), std::end(rng2));
}

}  // namespace step::longest_common_substring

#endif  // STEP_LONGEST_COMMON_SUBSTRING_HPP
