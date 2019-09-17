// Andrew Naplavkov

#ifndef STEP_TEST_SUFFIX_HPP
#define STEP_TEST_SUFFIX_HPP

//#include <boost/container/flat_map.hpp>
#include <array>
#include <fstream>
#include <random>
#include <step/example/suffix_tree_viz/graphviz.hpp>
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

        {"abcabxabcd$", R"(
digraph "abcabxabcd$" {
node_0 [shape=point]
10 [shape=plaintext]
node_0->10 [label="$"]
node_1 [shape=point]
node_0->node_1 [label="ab"]
node_3 [shape=point]
node_1->node_3 [label="c"]
0 [shape=plaintext]
node_3->0 [label="abxabcd$"]
6 [shape=plaintext]
node_3->6 [label="d$"]
3 [shape=plaintext]
node_1->3 [label="xabcd$"]
node_2 [shape=point]
node_0->node_2 [label="b"]
node_4 [shape=point]
node_2->node_4 [label="c"]
1 [shape=plaintext]
node_4->1 [label="abxabcd$"]
7 [shape=plaintext]
node_4->7 [label="d$"]
4 [shape=plaintext]
node_2->4 [label="xabcd$"]
node_5 [shape=point]
node_0->node_5 [label="c"]
2 [shape=plaintext]
node_5->2 [label="abxabcd$"]
8 [shape=plaintext]
node_5->8 [label="d$"]
9 [shape=plaintext]
node_0->9 [label="d$"]
5 [shape=plaintext]
node_0->5 [label="xabcd$"]
node_3->node_4 [style=dashed,arrowhead=otriangle]
node_1->node_2 [style=dashed,arrowhead=otriangle]
node_4->node_5 [style=dashed,arrowhead=otriangle]
}
)"},

        {"BANANA$", R"(
digraph "BANANA$" {
node_0 [shape=point]
6 [shape=plaintext]
node_0->6 [label="$"]
node_3 [shape=point]
node_0->node_3 [label="A"]
5 [shape=plaintext]
node_3->5 [label="$"]
node_1 [shape=point]
node_3->node_1 [label="NA"]
3 [shape=plaintext]
node_1->3 [label="$"]
1 [shape=plaintext]
node_1->1 [label="NA$"]
0 [shape=plaintext]
node_0->0 [label="BANANA$"]
node_2 [shape=point]
node_0->node_2 [label="NA"]
4 [shape=plaintext]
node_2->4 [label="$"]
2 [shape=plaintext]
node_2->2 [label="NA$"]
node_1->node_2 [style=dashed,arrowhead=otriangle]
node_2->node_3 [style=dashed,arrowhead=otriangle]
}
)"},

        {"xabxa$", R"(
digraph "xabxa$" {
node_0 [shape=point]
5 [shape=plaintext]
node_0->5 [label="$"]
node_2 [shape=point]
node_0->node_2 [label="a"]
4 [shape=plaintext]
node_2->4 [label="$"]
1 [shape=plaintext]
node_2->1 [label="bxa$"]
2 [shape=plaintext]
node_0->2 [label="bxa$"]
node_1 [shape=point]
node_0->node_1 [label="xa"]
3 [shape=plaintext]
node_1->3 [label="$"]
0 [shape=plaintext]
node_1->0 [label="bxa$"]
node_1->node_2 [style=dashed,arrowhead=otriangle]
}
)"}

    };
    for (auto& [str, expect] : tests) {
        std::ostringstream os;
        os << "\n" << graphviz{str};
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

inline std::string read_file(const char* file_name)
{
    using iterator_t = std::istreambuf_iterator<char>;
    std::ifstream stream{file_name};
    return {(iterator_t(stream)), iterator_t()};
}

inline std::string generate_text(size_t len)
{
    static std::mt19937 gen{std::random_device{}()};
    static std::array files = {"../longest_common_substring.hpp",
                               "../longest_repeated_substring.hpp",
                               "../suffix_array.hpp",
                               "../suffix_tree.hpp"};
    static std::uniform_int_distribution<size_t> dist{0, files.size() - 1};

    std::string res;
    while (res.size() < len) {
        res += read_file(files[dist(gen)]);
    }
    res.resize(len);
    res.back() = '\0';
    return res;
}

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
    for (size_t i = 1; i <= 4; ++i) {
        auto str = generate_text(i * 10000);
        step::suffix_array arr{str};
        ordered_suffix_tree tree{};
        tree.reserve(str.size());
        std::copy(str.begin(), str.end(), std::back_inserter(tree));
        CHECK(array_order(arr) == tree_order(tree));
    }
}

TEST_CASE("suffix_array_n_tree_benchmark")
{
    for (size_t exp = 17; exp < 22; ++exp) {
        auto str = generate_text((size_t)std::exp2(exp));

        BENCHMARK("2^" + std::to_string(exp) + " suffix array")
        {
            step::suffix_array<char, uint32_t> arr{str};
        }

        BENCHMARK("2^" + std::to_string(exp) + " suffix tree")
        {
            step::suffix_tree<char, uint32_t> tree{};
            tree.reserve((uint32_t)str.size());
            std::copy(str.begin(), str.end(), std::back_inserter(tree));
        }
    }
}

#endif  // STEP_TEST_SUFFIX_HPP
