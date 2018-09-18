// Andrew Naplavkov

#ifndef STEP_TEST_SUFFIX_TREE_HPP
#define STEP_TEST_SUFFIX_TREE_HPP

#include <chrono>
#include <cstdint>
#include <iterator>
#include <map>
#include <step/suffix_tree.hpp>
#include <step/test/utility.hpp>
#include <string_view>

TEST_CASE("suffix_tree_hello_world")
{
    using namespace std::literals;
    auto str = "use the quick find feature to search for a text"sv;
    step::suffix_tree tree{};
    std::copy(str.begin(), str.end(), std::back_inserter(tree));
    CHECK(tree.find("quick"sv) == 8);
}

inline auto make_suffix_tree(std::string_view str)
{
    step::suffix_tree<char, uint16_t, std::map> result;
    std::copy(str.begin(), str.end(), std::back_inserter(result));
    return result;
}

template <typename SuffixTree>
std::string to_string(const SuffixTree& tree)
{
    std::ostringstream os;
    tree.visit(
        [&](const auto& str, const auto&, auto len) {
            os << std::setw(len) << std::setfill(' ')
               << std::string_view{tree.begin(str), tree.size(str)};
            if (tree.suffix(str))
                os << " [" << str.second - len << "]";
            os << "\n";
        },
        [](auto&&...) {});
    return os.str();
}

TEST_CASE("suffix_tree_topology_n_find")
{
    struct {
        std::string_view str;
        std::string_view expected;
    } tests[] = {

        {"", ""},

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

    for (auto& [str, expected] : tests) {
        auto tree = make_suffix_tree(str);
        CHECK(to_string(tree) == expected);
        CHECK(tree.find(str) == 0);
        for (size_t i = 0; i < str.size(); ++i)
            CHECK(tree.find(str.begin() + i, str.end()) == i);
    }
}

TEST_CASE("suffix_tree_find_all")
{
    struct {
        std::string_view str;
        std::string_view pattern;
        std::initializer_list<uint16_t> expected;
    } tests[] = {
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
    for (auto& [str, pattern, expected] : tests) {
        auto tree = make_suffix_tree(str);
        auto offsets = tree.find_all(pattern);
        CHECK(std::is_permutation(
            offsets.begin(), offsets.end(), expected.begin(), expected.end()));
    }
}
/*
#include <boost/container/flat_map.hpp>
#include <boost/container/small_vector.hpp>

template <typename Key, typename T>
using flat_map = boost::container::flat_map<
    Key,
    T,
    std::less<>,
    boost::container::small_vector<std::pair<Key, T>, 8>>;
*/
TEST_CASE("suffix_tree_complexity")
{
    using namespace std::chrono;
    for (size_t i = 18; i < 22; ++i) {
        auto str = make_random_string(pow(2, i));
        str.back() = '$';
        step::suffix_tree<char, uint32_t /*, flat_map*/> tree;
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
