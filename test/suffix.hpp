// Andrew Naplavkov

#ifndef STEP_TEST_SUFFIX_HPP
#define STEP_TEST_SUFFIX_HPP

//#include <boost/container/flat_map.hpp>
#include <array>
#include <fstream>
#include <map>
#include <random>
#include <step/example/suffix_tree_viz/utility.hpp>
#include <step/suffix_array.hpp>
#include <step/suffix_tree.hpp>
#include <string>
#include <string_view>

using namespace std::literals;

TEST_CASE("suffix_array_hello_world")
{
    auto str = "how can I quickly search for text within a document?"sv;
    step::suffix_array arr{str};
    CHECK(arr.find("quick"sv) == 10);
}

TEST_CASE("suffix_tree_hello_world")
{
    auto str = "use the quick find feature to search for a text"sv;
    step::suffix_tree tree{};
    std::copy(str.begin(), str.end(), std::back_inserter(tree));
    CHECK(tree.find("quick"sv) == 8);
}

TEST_CASE("suffix_tree_graphviz")
{
    struct {
        std::string_view str;
        std::string_view expect;
    } tests[] = {

        {"abcabxabcd$", R"(digraph "abcabxabcd$" {
rankdir=LR
_0 [shape=point]
10 [shape=plaintext]
_0->10 [label="$"]
_1 [shape=point]
_0->_1 [label="ab"]
_3 [shape=point]
_1->_3 [label="c"]
0 [shape=plaintext]
_3->0 [label="abxabcd$"]
6 [shape=plaintext]
_3->6 [label="d$"]
3 [shape=plaintext]
_1->3 [label="xabcd$"]
_2 [shape=point]
_0->_2 [label="b"]
_4 [shape=point]
_2->_4 [label="c"]
1 [shape=plaintext]
_4->1 [label="abxabcd$"]
7 [shape=plaintext]
_4->7 [label="d$"]
4 [shape=plaintext]
_2->4 [label="xabcd$"]
_5 [shape=point]
_0->_5 [label="c"]
2 [shape=plaintext]
_5->2 [label="abxabcd$"]
8 [shape=plaintext]
_5->8 [label="d$"]
9 [shape=plaintext]
_0->9 [label="d$"]
5 [shape=plaintext]
_0->5 [label="xabcd$"]
}
)"},

        {"BANANA$", R"(digraph "BANANA$" {
rankdir=LR
_0 [shape=point]
6 [shape=plaintext]
_0->6 [label="$"]
_3 [shape=point]
_0->_3 [label="A"]
5 [shape=plaintext]
_3->5 [label="$"]
_1 [shape=point]
_3->_1 [label="NA"]
3 [shape=plaintext]
_1->3 [label="$"]
1 [shape=plaintext]
_1->1 [label="NA$"]
0 [shape=plaintext]
_0->0 [label="BANANA$"]
_2 [shape=point]
_0->_2 [label="NA"]
4 [shape=plaintext]
_2->4 [label="$"]
2 [shape=plaintext]
_2->2 [label="NA$"]
}
)"},

        {"xabxa$", R"(digraph "xabxa$" {
rankdir=LR
_0 [shape=point]
5 [shape=plaintext]
_0->5 [label="$"]
_2 [shape=point]
_0->_2 [label="a"]
4 [shape=plaintext]
_2->4 [label="$"]
1 [shape=plaintext]
_2->1 [label="bxa$"]
2 [shape=plaintext]
_0->2 [label="bxa$"]
_1 [shape=point]
_0->_1 [label="xa"]
3 [shape=plaintext]
_1->3 [label="$"]
0 [shape=plaintext]
_1->0 [label="bxa$"]
}
)"}

    };
    for (auto& [str, expect] : tests) {
        ordered_suffix_tree tree{};
        std::copy(str.begin(), str.end(), std::back_inserter(tree));
        std::ostringstream os;
        os << graphviz{tree};
        CHECK(os.str() == expect);
    }
}

TEST_CASE("suffix_array_n_tree_find")
{
    struct {
        std::string_view str;
        std::string_view pattern;
        std::initializer_list<size_t> expect;
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
    for (auto& [str, pattern, expect] : tests) {
        step::suffix_array arr{str};
        CHECK(arr.find(str) == 0);
        CHECK(arr.find("not found"sv) == arr.size());
        auto arr_all = arr.find_all(pattern);
        CHECK(std::is_permutation(
            arr_all.first, arr_all.second, expect.begin(), expect.end()));

        step::suffix_tree tree{};
        std::copy(str.begin(), str.end(), std::back_inserter(tree));
        CHECK(tree.find(str) == 0);
        CHECK(tree.find(""sv) == 0);
        CHECK(tree.find("not found"sv) == tree.size());
        std::vector<size_t> tree_all;
        tree.find_all(pattern, std::back_inserter(tree_all));
        CHECK(std::is_permutation(
            tree_all.begin(), tree_all.end(), expect.begin(), expect.end()));
    }
}

static const auto texts = [] {
    using iter_t = std::istreambuf_iterator<char>;
    std::mt19937 gen{std::random_device{}()};
    std::array files = {"../longest_common_substring.hpp",
                        "../longest_repeated_substring.hpp",
                        "../suffix_array.hpp",
                        "../suffix_tree.hpp"};
    std::uniform_int_distribution<size_t> dist{0, files.size() - 1};
    std::vector<std::string> res;
    for (size_t exp = 15; exp <= 17; ++exp) {
        auto& str = res.emplace_back();
        auto len = (size_t)std::exp2(exp);
        while (str.size() < len) {
            std::ifstream is{files[dist(gen)]};
            str.append((iter_t(is)), iter_t());
        }
        str.resize(len);
        str.back() = '\0';
    }
    return res;
}();

inline auto tree_order(const ordered_suffix_tree& tree)
{
    std::vector<size_t> res;
    tree.visit([&](auto& edge) {
        if (tree.leaf(edge.child))
            res.push_back(tree.path(edge).first);
    });
    return res;
}

inline auto array_order(const step::suffix_array<>& arr)
{
    std::vector<size_t> res(arr.size());
    for (size_t i = 0; i < arr.size(); ++i)
        res[i] = arr.nth_element(i);
    return res;
}

TEST_CASE("suffix_array_n_tree_cross_check")
{
    for (auto& str : texts) {
        step::suffix_array arr{str};
        ordered_suffix_tree tree{};
        std::copy(str.begin(), str.end(), std::back_inserter(tree));
        CHECK(array_order(arr) == tree_order(tree));
    }
}

TEST_CASE("suffix_array_benchmark")
{
    for (auto& str : texts)
        BENCHMARK(std::to_string(str.size()) + " chars suffix array")
        {
            step::suffix_array<char, uint32_t> arr{str};
        };
}

TEST_CASE("suffix_tree_benchmark")
{
    for (auto& str : texts)
        BENCHMARK(std::to_string(str.size()) + " chars suffix tree")
        {
            step::suffix_tree<char, uint32_t, std::map> tree{};
            tree.reserve((uint32_t)str.size());
            std::copy(str.begin(), str.end(), std::back_inserter(tree));
        };
}

#endif  // STEP_TEST_SUFFIX_HPP
