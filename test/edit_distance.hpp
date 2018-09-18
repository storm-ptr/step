// Andrew Naplavkov

#ifndef STEP_TEST_EDIT_DISTANCE_HPP
#define STEP_TEST_EDIT_DISTANCE_HPP

#include <step/edit_distance.hpp>
#include <step/test/utility.hpp>
#include <string_view>

using option_t = std::optional<char>;
using pairs_t = std::vector<std::pair<option_t, option_t>>;

TEST_CASE("edit_distance_hello_world")
{
    using namespace std::literals;
    pairs_t pairs;
    step::edit_distance::join("this"sv, "has"sv, std::back_inserter(pairs));
    CHECK(pairs ==
          pairs_t{{'t', std::nullopt}, {'h', 'h'}, {'i', 'a'}, {'s', 's'}});
}

TEST_CASE("edit_distance_associate")
{
    struct {
        std::string_view lhs;
        std::string_view rhs;
        pairs_t expected;
    } tests[] = {{"SUNDAY",
                  "saturday",
                  pairs_t{{'S', 's'},
                          {std::nullopt, 'a'},
                          {std::nullopt, 't'},
                          {'U', 'u'},
                          {'N', 'r'},
                          {'D', 'd'},
                          {'A', 'a'},
                          {'Y', 'y'}}},
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
                          {'C', 'C'}}},
                 {"Hyundai",
                  "Honda",
                  pairs_t{{'H', 'H'},
                          {'y', std::nullopt},
                          {'u', 'o'},
                          {'n', 'n'},
                          {'d', 'd'},
                          {'a', 'a'},
                          {'i', std::nullopt}}}};
    for (auto& [lhs, rhs, expected] : tests) {
        pairs_t pairs;
        step::edit_distance::join(
            lhs, rhs, std::back_inserter(pairs), case_insensitive_equal_to{});
        CHECK(pairs == expected);
    }
}

#endif  // STEP_TEST_EDIT_DISTANCE_HPP
