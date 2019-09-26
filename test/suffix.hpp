// Andrew Naplavkov

#ifndef STEP_TEST_SUFFIX_HPP
#define STEP_TEST_SUFFIX_HPP

//#include <boost/container/flat_map.hpp>
#include <array>
#include <fstream>
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
0_ [shape=point]
10 [shape=plaintext]
0_->10 [label="$"]
1_ [shape=point]
0_->1_ [label="ab"]
3_ [shape=point]
1_->3_ [label="c"]
0 [shape=plaintext]
3_->0 [label="abxabcd$"]
6 [shape=plaintext]
3_->6 [label="d$"]
3 [shape=plaintext]
1_->3 [label="xabcd$"]
2_ [shape=point]
0_->2_ [label="b"]
4_ [shape=point]
2_->4_ [label="c"]
1 [shape=plaintext]
4_->1 [label="abxabcd$"]
7 [shape=plaintext]
4_->7 [label="d$"]
4 [shape=plaintext]
2_->4 [label="xabcd$"]
5_ [shape=point]
0_->5_ [label="c"]
2 [shape=plaintext]
5_->2 [label="abxabcd$"]
8 [shape=plaintext]
5_->8 [label="d$"]
9 [shape=plaintext]
0_->9 [label="d$"]
5 [shape=plaintext]
0_->5 [label="xabcd$"]
}
)"},

        {"BANANA$", R"(digraph "BANANA$" {
rankdir=LR
0_ [shape=point]
6 [shape=plaintext]
0_->6 [label="$"]
3_ [shape=point]
0_->3_ [label="A"]
5 [shape=plaintext]
3_->5 [label="$"]
1_ [shape=point]
3_->1_ [label="NA"]
3 [shape=plaintext]
1_->3 [label="$"]
1 [shape=plaintext]
1_->1 [label="NA$"]
0 [shape=plaintext]
0_->0 [label="BANANA$"]
2_ [shape=point]
0_->2_ [label="NA"]
4 [shape=plaintext]
2_->4 [label="$"]
2 [shape=plaintext]
2_->2 [label="NA$"]
}
)"},

        {"xabxa$", R"(digraph "xabxa$" {
rankdir=LR
0_ [shape=point]
5 [shape=plaintext]
0_->5 [label="$"]
2_ [shape=point]
0_->2_ [label="a"]
4 [shape=plaintext]
2_->4 [label="$"]
1 [shape=plaintext]
2_->1 [label="bxa$"]
2 [shape=plaintext]
0_->2 [label="bxa$"]
1_ [shape=point]
0_->1_ [label="xa"]
3 [shape=plaintext]
1_->3 [label="$"]
0 [shape=plaintext]
1_->0 [label="bxa$"]
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

static const auto TEXTS = [] {
    using iter_t = std::istreambuf_iterator<char>;
    static std::mt19937 gen{std::random_device{}()};
    static std::array files = {"../longest_common_substring.hpp",
                               "../longest_repeated_substring.hpp",
                               "../suffix_array.hpp",
                               "../suffix_tree.hpp",
                               "./suffix.hpp"};
    static std::uniform_int_distribution<size_t> dist{0, files.size() - 1};

    std::vector<std::string> res;
    for (size_t exp = 15; exp <= 17; ++exp) {
        auto len = (size_t)std::exp2(exp);
        auto& str = res.emplace_back();
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
    res.reserve(tree.size());
    tree.visit([&](auto& edge) {
        if (tree.leaf(edge.child))
            res.push_back(tree.path(edge).first);
    });
    return res;
}

template <class Array>
auto array_order(const Array& arr)
{
    using size_type = typename Array::size_type;
    std::vector<size_type> res(arr.size());
    for (size_type i = 0; i < arr.size(); ++i)
        res[i] = arr.nth_element(i);
    return res;
}

TEST_CASE("suffix_array_n_tree_cross_check")
{
    for (auto& str : TEXTS) {
        step::suffix_array arr{str};
        ordered_suffix_tree tree{};
        std::copy(str.begin(), str.end(), std::back_inserter(tree));
        CHECK(array_order(arr) == tree_order(tree));
    }
}

TEST_CASE("suffix_array_benchmark")
{
    for (auto& str : TEXTS) {
        BENCHMARK(std::to_string(str.size()) + " chars suffix array")
        {
            step::suffix_array<char, uint32_t> arr{str};
        };
    }
}

TEST_CASE("suffix_tree_benchmark")
{
    for (auto& str : TEXTS) {
        BENCHMARK(std::to_string(str.size()) + " chars suffix tree")
        {
            step::suffix_tree<char, uint32_t> tree{};
            tree.reserve((uint32_t)str.size());
            std::copy(str.begin(), str.end(), std::back_inserter(tree));
        };
    }
}

#endif  // STEP_TEST_SUFFIX_HPP
