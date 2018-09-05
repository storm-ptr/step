// Andrew Naplavkov

#ifndef STEP_TEST_SUFFIX_TREE_HPP
#define STEP_TEST_SUFFIX_TREE_HPP

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <random>
#include <step/suffix_tree.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

inline std::string random_string(size_t len)
{
    static const char Alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    static std::mt19937 generator{std::random_device{}()};
    static std::uniform_int_distribution<size_t> distribution{
        0, sizeof(Alphanum) - 2};
    std::string result;
    result.reserve(len);
    for (size_t i = 0; i < len; ++i)
        result.push_back(Alphanum[distribution(generator)]);
    result.back() = '$';
    return result;
}

inline auto make_suffix_tree(std::string_view str)
{
    step::suffix_tree<char> result{};
    std::copy(str.begin(), str.end(), std::back_inserter(result));
    return result;
}

template <typename SuffixTree>
std::string to_string(const SuffixTree& tree)
{
    std::ostringstream os;
    tree.visit([&](auto first, auto nodal, auto last) {
        os << std::setw(last - first) << std::setfill(' ')
           << std::string_view{tree.data() + nodal, last - nodal};
        if (last == tree.size())  // suffix
            os << " [" << first << "]";
        os << "\n";
    });
    return os.str();
}

TEST_CASE("suffix_tree_topology")
{
    struct {
        std::string_view str;
        std::string_view expected;
    } CASES[] = {

        {"abcabxabcd$", R"(
$ [10]
ab
  c
   abxabcd$ [0]
   d$ [6]
  xabcd$ [3]
b
 c
  abxabcd$ [1]
  d$ [7]
 xabcd$ [4]
c
 abxabcd$ [2]
 d$ [8]
d$ [9]
xabcd$ [5]
)"},

        {"BANANA$", R"(
$ [6]
A
 $ [5]
 NA
   $ [3]
   NA$ [1]
BANANA$ [0]
NA
  $ [4]
  NA$ [2]
)"},

        {"VVuVVVOm$", R"(
$ [8]
Om$ [6]
V
 Om$ [5]
 V
  Om$ [4]
  VOm$ [3]
  uVVVOm$ [0]
 uVVVOm$ [1]
m$ [7]
uVVVOm$ [2]
)"}};

    for (auto & [ str, expected ] : CASES)
        CHECK(to_string(make_suffix_tree(str)) == expected);
}

TEST_CASE("suffix_tree_find")
{
    for (std::string_view str : {"abcabxabcd$", "BANANA$", "VVuVVVOm$"}) {
        auto tree = make_suffix_tree(str);
        CHECK(tree.find(str.begin(), str.begin()) == 0);
        for (size_t i = 0; i < str.size(); ++i)
            CHECK(tree.find(str.begin() + i, str.end()) == i);
    }
}

TEST_CASE("suffix_tree_find_all")
{
    struct {
        std::string_view str;
        std::string_view pattern;
        std::initializer_list<size_t> expected;
    } CASES[] = {
        {"GEEKSFORGEEKS$", "GEEKS", {0, 8}},
        {"GEEKSFORGEEKS$", "GEEK1", {}},
        {"GEEKSFORGEEKS$", "FOR", {5}},
        {"AABAACAADAABAAABAA$", "AABA", {0, 9, 13}},
        {"AABAACAADAABAAABAA$", "AA", {0, 3, 6, 9, 12, 13, 16}},
        {"AABAACAADAABAAABAA$", "AAE", {}},
        {"AAAAAAAAA$", "AAAA", {0, 1, 2, 3, 4, 5}},
        {"AAAAAAAAA$", "AA", {0, 1, 2, 3, 4, 5, 6, 7}},
        {"AAAAAAAAA$", "A", {0, 1, 2, 3, 4, 5, 6, 7, 8}},
        {"AAAAAAAAA$", "AB", {}},
    };
    for (auto & [ str, pattern, expected ] : CASES) {
        auto tree = make_suffix_tree(str);
        auto offsets = tree.find_all(pattern.begin(), pattern.end());
        CHECK(std::is_permutation(
            offsets.begin(), offsets.end(), expected.begin(), expected.end()));
    }
}
/*
#include <boost/container/flat_map.hpp>
#include <boost/container/small_vector.hpp>

template <typename Key, typename T>
using map_t = boost::container::flat_map<
    Key,
    T,
    std::less<Key>,
    boost::container::small_vector<std::pair<Key, T>, 8>>;
*/
TEST_CASE("suffix_tree_complexity")
{
    using namespace std::chrono;
    for (size_t i = 18; i < 22; ++i) {
        auto str = random_string(pow(2, i));
        step::suffix_tree<char, std::unordered_map, uint32_t> tree{};
        tree.reserve(str.size());
        auto start = high_resolution_clock::now();
        std::copy(str.begin(), str.end(), std::back_inserter(tree));
        auto elapsed =
            duration_cast<milliseconds>(high_resolution_clock::now() - start)
                .count();
        std::cout << "suffix_tree construction (" << str.size()
                  << " chars) took: " << elapsed << " ms\n";
    }
}

#endif  // STEP_TEST_SUFFIX_TREE_HPP
