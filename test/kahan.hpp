// Andrew Naplavkov

#ifndef STEP_TEST_KAHAN_HPP
#define STEP_TEST_KAHAN_HPP

#include <functional>
#include <limits>
#include <step/detail/utility.hpp>
#include <step/kahan.hpp>

template <class InputIt, class BinaryOp>
auto sum(InputIt first, InputIt last, BinaryOp op)
{
    decltype(op(*first, *first)) result{};
    for (; first != last; ++first)
        result = op(result, *first);
    return static_cast<step::iterator_value<InputIt>>(result);
}

TEST_CASE("kahan")
{
    long double eps = std::numeric_limits<long double>::epsilon();
    long double quarter = eps / 4.;
    long double arr[] = {1., quarter, quarter, quarter, quarter};
    long double a = sum(std::begin(arr), std::end(arr), std::plus{});
    long double b = sum(std::begin(arr), std::end(arr), step::kahan::plus{});
    CHECK(a == 1.);
    CHECK(b == 1. + eps);
}

#endif  // STEP_TEST_KAHAN_HPP
