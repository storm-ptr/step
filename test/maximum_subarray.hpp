// Andrew Naplavkov

#ifndef STEP_TEST_MAXIMUM_SUBARRAY_HPP
#define STEP_TEST_MAXIMUM_SUBARRAY_HPP

#include <array>
#include <step/kahan.hpp>
#include <step/maximum_subarray.hpp>
#include <vector>

TEST_CASE("maximum_subarray_hello_world")
{
    int arr[] = {-2, -3, 4, -1, -2, 1, 5, -3};
    std::array expect{4, -1, -2, 1, 5};
    auto range = step::maximum_subarray::find(arr);
    CHECK(std::equal(range.first, range.second, expect.begin(), expect.end()));
}

TEST_CASE("maximum_subarray_find")
{
    struct {
        std::vector<int> arr;
        std::vector<int> expect;
    } tests[] = {
        {{-2, 1, -3, 4, -1, 2, 1, -5, 4}, {4, -1, 2, 1}},
        {{2, 3, -1, -20, 5, 10}, {5, 10}},
        {{-1, -2, 3, 5, 6, -2, -1, 4, -4, 2, -1}, {3, 5, 6, -2, -1, 4}},
        {{-1, -2, -3, -4, -5}, {-1}},
        {{7, -6, -8, 5, -2, -6, 7, 4, 8, -9, -3, 2, 6, -4, -6}, {7, 4, 8}},
        {{0, 1, 2, -3, 3, -1, 0, -4, 0, -1, -4, 2}, {1, 2}}};
    for (auto& [arr, expect] : tests) {
        auto rng = step::maximum_subarray::find(arr);
        CHECK(std::vector<int>(rng.first, rng.second) == expect);
    }
}

TEST_CASE("maximum_subarray_kahan")
{
    using namespace step::maximum_subarray;
    std::vector<float> v{1.};
    v.insert(v.end(), 10000000, 0.0000001f);
    auto rng = find(v);
    auto kahan_rng = find(v, step::kahan::plus{}, std::less{});
    CHECK((size_t)step::size(rng) < v.size());
    CHECK((size_t)step::size(kahan_rng) == v.size());
}

#endif  // STEP_TEST_MAXIMUM_SUBARRAY_HPP
