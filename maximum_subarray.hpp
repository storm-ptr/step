// Andrew Naplavkov

#ifndef STEP_MAXIMUM_SUBARRAY_HPP
#define STEP_MAXIMUM_SUBARRAY_HPP

#include <iterator>
#include <utility>

namespace step {
namespace maximum_subarray {
namespace detail {

template <typename T, typename ForwardIt>
struct weighted_range {
    T weight;
    ForwardIt first;
    ForwardIt last;
};

template <typename T, typename ForwardIt>
auto make_weighted_range(ForwardIt it)
{
    return weighted_range<T, ForwardIt>{*it, it, std::next(it)};
}

}  // namespace detail

/**
 * Find the bounds of the contiguous subrange which has the largest sum.
 *
 * Time complexity O(N), space complexity O(1),
 * where N = std::distance(first, last).
 */
template <typename ForwardIt, typename BinaryOp, typename Compare>
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

template <typename ForwardIt>
auto find(ForwardIt first, ForwardIt last)
{
    return maximum_subarray::find(first, last, std::plus{}, std::less{});
}

template <typename ForwardRng, typename BinaryOp, typename Compare>
auto find(const ForwardRng& rng, BinaryOp op, Compare cmp)
{
    return maximum_subarray::find(std::begin(rng), std::end(rng), op, cmp);
}

template <typename ForwardRng>
auto find(const ForwardRng& rng)
{
    return maximum_subarray::find(std::begin(rng), std::end(rng));
}

}  // namespace maximum_subarray
}  // namespace step

#endif  // STEP_MAXIMUM_SUBARRAY_HPP