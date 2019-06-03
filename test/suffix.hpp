// Andrew Naplavkov

#ifndef STEP_TEST_SUFFIX_HPP
#define STEP_TEST_SUFFIX_HPP

//#include <boost/container/flat_map.hpp>
#include <map>
#include <random>
#include <step/suffix_array.hpp>
#include <step/suffix_tree.hpp>
#include <string_view>

using namespace std::literals;

template <class Key, class T>
using reverse_ordered_map = std::map<Key, T, std::greater<>>;

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

template <class Tree>
std::string tree_topology(const Tree& tree)
{
    std::ostringstream os;
    tree.visit([&](auto& edge) {
        if (edge.visited)
            return;
        auto rng = tree.label(edge.child);
        os << std::setw(edge.path) << std::setfill(' ')
           << std::string_view{tree.begin(rng), step::size(rng)};
        if (tree.leaf(edge.child))
            os << " [" << tree.path(edge).first << "]";
        os << "\n";
    });
    return os.str();
}

template <class Tree>
auto tree_order(const Tree& tree)
{
    std::vector<typename Tree::size_type> res;
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

TEST_CASE("suffix_tree_topology")
{
    struct {
        std::string_view str;
        std::string_view expect;
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
)"},

        {"wwwJwww$", R"(
$ [7]
Jwww$ [3]
w
 $ [6]
 Jwww$ [2]
 w
  $ [5]
  Jwww$ [1]
  w
   $ [4]
   Jwww$ [0]
)"},
    };

    for (auto& [str, expect] : tests) {
        step::suffix_tree<char, size_t, reverse_ordered_map> tree{};
        std::copy(str.begin(), str.end(), std::back_inserter(tree));
        CHECK(tree_topology(tree) == expect);
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

inline std::string make_random_string(size_t len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    static std::mt19937 gen{std::random_device{}()};
    static std::uniform_int_distribution<size_t> dist{0, sizeof(alphanum) - 2};
    std::string res(len, '\0');
    std::generate(res.begin(), res.end(), [&] { return alphanum[dist(gen)]; });
    return res;
}

TEST_CASE("suffix_array_n_tree_cross_check")
{
    for (size_t i = 0; i < 100; ++i) {
        auto str = make_random_string(10000);
        str += str;
        str.back() = '$';

        step::suffix_array<char, uint16_t> arr{str};
        step::suffix_tree<char, uint16_t, reverse_ordered_map> tree{};
        tree.reserve((uint16_t)str.size());
        std::copy(str.begin(), str.end(), std::back_inserter(tree));

        CHECK(array_order(arr) == tree_order(tree));

        for (size_t j = 2; j <= 4; ++j) {
            auto pattern = make_random_string(j);
            auto arr_all = arr.find_all(pattern);
            std::vector<uint16_t> tree_all;
            tree.find_all(pattern, std::back_inserter(tree_all));
            CHECK(std::is_permutation(arr_all.first,
                                      arr_all.second,
                                      tree_all.begin(),
                                      tree_all.end()));
        }
    }
}

TEST_CASE("suffix_array_n_tree_benchmark")
{
    for (size_t exp = 17; exp < 21; ++exp) {
        auto str = make_random_string((size_t)std::exp2(exp));
        auto repeat = make_random_string((size_t)std::exp2(exp) / 2);
        repeat += repeat;
        str.back() = repeat.back() = '$';
        auto size = "2^" + std::to_string(exp);

        BENCHMARK(size + " suffix array")
        {
            step::suffix_array<char, uint32_t> arr{str};
        }

        BENCHMARK(size + " suffix array (repeat)")
        {
            step::suffix_array<char, uint32_t> arr{repeat};
        }

        BENCHMARK(size + " suffix tree")
        {
            step::suffix_tree<char, uint32_t> tree{};
            tree.reserve((uint32_t)str.size());
            std::copy(str.begin(), str.end(), std::back_inserter(tree));
        }

        BENCHMARK(size + " suffix tree (repeat)")
        {
            step::suffix_tree<char, uint32_t> tree{};
            tree.reserve((uint32_t)repeat.size());
            std::copy(repeat.begin(), repeat.end(), std::back_inserter(tree));
        }
    }
}

#endif  // STEP_TEST_SUFFIX_HPP
