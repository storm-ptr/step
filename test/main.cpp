// Andrew Naplavkov

#define CATCH_CONFIG_COUNTER
#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <array>
#include <cstring>
#include <deque>
#include <iterator>
#include <sstream>
#include <step/edit_distance.hpp>
#include <step/longest_common_subsequence.hpp>
#include <step/longest_increasing_subsequence.hpp>
#include <string>
#include <string_view>
#include <vector>

TEST_CASE("longest_common_subsequence")
{
    using namespace step::longest_common_subsequence;
    std::ostringstream os;
    intersect(
        std::string{"XMJYAUZ"}, "MZJAWXU", std::ostream_iterator<char>(os));
    REQUIRE(os.str() == "MJAU");

    std::string s;
    intersect("AGGTAB", std::string_view{"GXTXAYB"}, std::back_inserter(s));
    REQUIRE(s == "GTAB");

    std::vector<char> v;
    intersect("ABCDGH", "AEDFHR", std::back_inserter(v));
    REQUIRE(strcmp(v.data(), "ADH") == 0);
}

TEST_CASE("longest_increasing_subsequence")
{
    using namespace step::longest_increasing_subsequence;

    std::deque d{0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15};
    auto end_of_d = partition(d);
    std::ostringstream os;
    std::copy(d.begin(), end_of_d, std::ostream_iterator<int>(os, " "));
    REQUIRE(os.str() == "0 2 6 9 11 15 ");

    std::string s{"CBFDEA"};
    auto end_of_s = partition(s);
    REQUIRE(std::string_view(s.c_str(), std::distance(s.begin(), end_of_s)) ==
            "BDE");

    int a[] = {10, 22, 9, 33, 21, 50, 41, 60};
    REQUIRE(std::vector<int>(std::begin(a), partition(a)) ==
            std::vector{10, 22, 33, 41, 60});

    std::vector perm{1, 1, 2, 2, 3, 3, 4, 4};
    do {
        std::vector lis(perm);
        auto end_of_lis = partition(lis);
        REQUIRE(std::is_permutation(lis.begin(), lis.end(), perm.begin()));
        REQUIRE(std::is_sorted(lis.begin(), end_of_lis));
        REQUIRE(std::distance(lis.begin(), end_of_lis) >= 2);
    } while (std::next_permutation(perm.begin(), perm.end()));
}

TEST_CASE("edit_distance")
{
    using namespace step::edit_distance;
    using option_t = std::optional<char>;
    using pairs_t = std::vector<std::pair<option_t, option_t>>;
    using test_t = std::tuple<std::string_view, std::string_view, pairs_t>;
    test_t Tests[] = {{"sunday",
                       "saturday",
                       pairs_t{{'s', 's'},
                               {std::nullopt, 'a'},
                               {std::nullopt, 't'},
                               {'u', 'u'},
                               {'n', 'r'},
                               {'d', 'd'},
                               {'a', 'a'},
                               {'y', 'y'}}},
                      {"GCGTATGAGGCTAACGC",
                       "GCTATGCGGCTATACGC",
                       pairs_t{{'G', 'G'},
                               {'C', 'C'},
                               {'G', std::nullopt},
                               {'T', 'T'},
                               {'A', 'A'},
                               {'T', 'T'},
                               {'G', 'G'},
                               {'A', 'C'},
                               {'G', 'G'},
                               {'G', 'G'},
                               {'C', 'C'},
                               {'T', 'T'},
                               {'A', 'A'},
                               {std::nullopt, 'T'},
                               {'A', 'A'},
                               {'C', 'C'},
                               {'G', 'G'},
                               {'C', 'C'}}}};
    for (const auto& test : Tests) {
        pairs_t pairs;
        align(std::get<0>(test), std::get<1>(test), std::back_inserter(pairs));
        REQUIRE(pairs == std::get<2>(test));
    }
}
