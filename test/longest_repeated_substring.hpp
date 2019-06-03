// Andrew Naplavkov

#ifndef STEP_TEST_LONGEST_REPEATED_SUBSTRING_HPP
#define STEP_TEST_LONGEST_REPEATED_SUBSTRING_HPP

#include <map>
#include <step/longest_repeated_substring.hpp>
#include <step/test/case_insensitive.hpp>
#include <string_view>

TEST_CASE("longest_repeated_substring_hello_world")
{
    auto range = step::longest_repeated_substring::find_with_suffix_array(
        "the longest substring of a string that occurs at least twice");
    CHECK("string " == std::string(range.first, range.second));
}

TEST_CASE("longest_repeated_substring_find")
{
    struct {
        std::string_view str;
        std::string_view expect;
    } tests[] = {
        {"GEEKSFORGEEKS$", "GEEKS"},
        {"AAAAAAAAAA$", "AAAAAAAAA"},
        {"ABCDEFG$", ""},
        {"ABABABA$", "ABABA"},
        {"ATCGATCGA$", "ATCGA"},
        {"banana$", "ana"},
        {"ATCGATCGA$", "ATCGA"},
        {"mississippi$", "issi"},
        {"abcabcaacb$", "abca"},
        {"aababa$", "aba"},
    };
    for (auto& [str, expect] : tests) {
        auto arr_rng =
            step::longest_repeated_substring::find_with_suffix_array(str);
        CHECK(expect == std::string(arr_rng.first, arr_rng.second));

        auto tree_rng =
            step::longest_repeated_substring::find_with_suffix_tree<std::map>(
                str);
        CHECK(expect == std::string(tree_rng.first, tree_rng.second));
    }
}

TEST_CASE("longest_repeated_substring_case_insensitive")
{
    const char str[] = "geeksForGeeks";
    const std::string expect = "geeks";

    auto arr_rng = step::longest_repeated_substring::find_with_suffix_array<
        step::case_insensitive::less>(str);
    CHECK(std::equal(expect.begin(),
                     expect.end(),
                     arr_rng.first,
                     arr_rng.second,
                     step::case_insensitive::equal_to{}));

    auto tree_rng = step::longest_repeated_substring::find_with_suffix_tree<
        step::case_insensitive::unordered_map>(str);
    CHECK(std::equal(expect.begin(),
                     expect.end(),
                     tree_rng.first,
                     tree_rng.second,
                     step::case_insensitive::equal_to{}));
}

#endif  // STEP_TEST_LONGEST_REPEATED_SUBSTRING_HPP
