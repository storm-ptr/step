// Andrew Naplavkov

#ifndef STEP_MAXIMUM_SUBARRAY_HPP
#define STEP_MAXIMUM_SUBARRAY_HPP

#include <iterator>
#include <utility>

namespace step::maximum_subarray {
namespace detail {

template <class T, class ForwardIt>
struct weighted_range {
    T weight;
    ForwardIt first;
    ForwardIt last;
};

template <class T, class ForwardIt>
auto make_weighted_range(ForwardIt it)
{
    return weighted_range<T, ForwardIt>{*it, it, std::next(it)};
}

}  // namespace detail

/// Kadane's algorithm.

/// Find the bounds of the contiguous subrange which has the largest sum.
/// Time complexity O(N), space complexity O(1), where:
/// N = std::distance(first, last).
/// @return a pair of iterators defining the wanted subarray.
/// @see https://en.wikipedia.org/wiki/Maximum_subarray_problem
template <class ForwardIt, class BinaryOp, class Compare>
std::pair<ForwardIt, ForwardIt> find(ForwardIt first,
                                     ForwardIt last,
                                     BinaryOp op,
                                     Compare cmp)
{
    using weight_t = decltype(op(*first, *first));
    if (first == last)
        return {first, last};
    auto rng = detail::make_weighted_range<weight_t>(first);
    auto result = rng;
    while (rng.last != last) {
        rng.weight = op(std::move(rng.weight), *rng.last);
        if (cmp(*rng.last, rng.weight))
            ++rng.last;
        else
            rng = detail::make_weighted_range<weight_t>(rng.last);
        if (cmp(result.weight, rng.weight))
            result = rng;
    }
    return {result.first, result.last};
}

template <class ForwardIt>
auto find(ForwardIt first, ForwardIt last)
{
    return maximum_subarray::find(first, last, std::plus{}, std::less{});
}

template <class ForwardRng, class BinaryOp, class Compare>
auto find(const ForwardRng& rng, BinaryOp&& op, Compare&& cmp)
{
    return maximum_subarray::find(std::begin(rng),
                                  std::end(rng),
                                  std::forward<BinaryOp>(op),
                                  std::forward<Compare>(cmp));
}

template <class ForwardRng>
auto find(const ForwardRng& rng)
{
    return maximum_subarray::find(std::begin(rng), std::end(rng));
}

}  // namespace step::maximum_subarray

#endif  // STEP_MAXIMUM_SUBARRAY_HPP
