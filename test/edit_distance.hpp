// Andrew Naplavkov

#ifndef STEP_TEST_EDIT_DISTANCE_HPP
#define STEP_TEST_EDIT_DISTANCE_HPP

#include <step/edit_distance.hpp>
#include <step/test/common.hpp>
#include <string_view>

TEST_CASE("edit_distance")
{
    using namespace step::edit_distance;
    using option_t = std::optional<char>;
    using pairs_t = std::vector<std::pair<option_t, option_t>>;
    using test_t = std::tuple<std::string_view, std::string_view, pairs_t>;
    test_t Tests[] = {{"SUNDAY",
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
                               {'C', 'C'}}}};
    for (const auto& test : Tests) {
        pairs_t pairs;
        associate(std::get<0>(test),
                  std::get<1>(test),
                  std::back_inserter(pairs),
                  iequals);
        CHECK(pairs == std::get<2>(test));
    }
}

#endif  // STEP_TEST_EDIT_DISTANCE_HPP
