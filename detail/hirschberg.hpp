// Andrew Naplavkov

#ifndef STEP_HIRSCHBERG_HPP
#define STEP_HIRSCHBERG_HPP

#include <algorithm>
#include <step/detail/utility.hpp>

namespace step {
namespace hirschberg {
namespace detail {

template <typename RandomIt1,
          typename RandomIt2,
          typename DynamicProg,
          typename BinaryOp>
auto partition_point(RandomIt1 first1,
                     RandomIt1 last1,
                     RandomIt2 first2,
                     RandomIt2 last2,
                     DynamicProg dp,
                     BinaryOp op)
{
    auto split1 = first1 + std::distance(first1, last1) / 2;
    auto top = dp.make_last_row(first1, split1, first2, last2);
    auto bottom = dp.make_last_row(std::make_reverse_iterator(last1),
                                   std::make_reverse_iterator(split1),
                                   std::make_reverse_iterator(last2),
                                   std::make_reverse_iterator(first2));
    std::transform(
        top.begin(), top.end(), bottom.rbegin(), top.begin(), std::plus{});
    auto split2 =
        first2 + std::distance(top.begin(),
                               std::min_element(top.begin(), top.end(), dp));
    return op(split1, split2);
}

}  // namespace detail

/// @see https://en.wikipedia.org/wiki/Hirschberg's_algorithm
template <typename RandomIt1,
          typename RandomIt2,
          typename OutputIt,
          typename DynamicProg>
OutputIt trace(RandomIt1 first1,
               RandomIt1 last1,
               RandomIt2 first2,
               RandomIt2 last2,
               OutputIt result,
               DynamicProg dp)
{
    auto size1 = std::distance(first1, last1);
    auto size2 = std::distance(first2, last2);
    if (size1 < 2 || size2 < 2)
        return dp.trivial_trace(first1, last1, first2, last2, result);

    auto [split1, split2] =
        size2 < size1
            ? detail::partition_point(
                  first1, last1, first2, last2, dp, make_pair{})
            : detail::partition_point(
                  first2, last2, first1, last1, dp, make_reverse_pair{});

    result = hirschberg::trace(first1, split1, first2, split2, result, dp);
    result = hirschberg::trace(split1, last1, split2, last2, result, dp);
    return result;
}

}  // namespace hirschberg
}  // namespace step

#endif  // STEP_HIRSCHBERG_HPP
