// Andrew Naplavkov

#ifndef STEP_TEST_KAHAN_HPP
#define STEP_TEST_KAHAN_HPP

#include <functional>
#include <limits>
#include <numeric>
#include <step/kahan.hpp>
#include <type_traits>

template <class T>
std::enable_if_t<std::is_floating_point_v<T>> test_kahan()
{
    T eps = std::numeric_limits<T>::epsilon();
    T quarter = eps / (T)4.;
    T arr[] = {(T)1., quarter, quarter, quarter, quarter};
    T acc = std::accumulate(std::begin(arr), std::end(arr), T{});
    T kahan_acc = (T)std::accumulate(
        std::begin(arr), std::end(arr), step::kahan::floating_point<T>{});
    CHECK((T)1. != (T)1. + eps);
    CHECK(acc == (T)1.);
    CHECK(kahan_acc == (T)1. + eps);
}

TEST_CASE("kahan")
{
    test_kahan<float>();
    test_kahan<double>();
    test_kahan<long double>();
}

#endif  // STEP_TEST_KAHAN_HPP
