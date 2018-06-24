// Andrew Naplavkov

#ifndef STEP_TEST_MAXIMUM_SUBARRAY_HPP
#define STEP_TEST_MAXIMUM_SUBARRAY_HPP

#include <array>
#include <step/kahan.hpp>
#include <step/maximum_subarray.hpp>
#include <vector>

TEST_CASE("maximum_subarray")
{
    using namespace step::maximum_subarray;

    int a[] = {-2, -3, 4, -1, -2, 1, 5, -3};
    auto sub_a = find(a);
    CHECK(std::vector<int>(sub_a.first, sub_a.second) ==
          std::vector{4, -1, -2, 1, 5});

    std::array arr = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    auto sub_arr = find(arr);
    CHECK(std::vector<int>(sub_arr.first, sub_arr.second) ==
          std::vector{4, -1, 2, 1});

    std::vector<float> v{1.};
    v.insert(v.end(), 10000000, 0.0000001);
    auto sub_v = find(v);
    auto kahan_sub_v = find(v, step::kahan::plus{}, std::less{});
    CHECK(std::distance(sub_v.first, sub_v.second) < v.size());
    CHECK(std::distance(kahan_sub_v.first, kahan_sub_v.second) == v.size());
}

#endif  // STEP_TEST_MAXIMUM_SUBARRAY_HPP
