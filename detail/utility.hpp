// Andrew Naplavkov

#ifndef STEP_UTILITY_HPP
#define STEP_UTILITY_HPP

#include <array>
#include <iterator>
#include <utility>
#include <vector>

namespace step {

template <class It>
using iterator_value_t = typename std::iterator_traits<It>::value_type;

template <class Rng>
using range_value_t = iterator_value_t<decltype(std::declval<Rng>().begin())>;

struct make_pair {
    template <typename Lhs, typename Rhs>
    auto operator()(Lhs&& lhs, Rhs&& rhs) const
    {
        return std::make_pair(std::forward<Lhs>(lhs), std::forward<Rhs>(rhs));
    }
};

struct make_reverse_pair {
    template <typename Lhs, typename Rhs>
    auto operator()(Lhs&& lhs, Rhs&& rhs) const
    {
        return std::make_pair(std::forward<Rhs>(rhs), std::forward<Lhs>(lhs));
    }
};

template <typename T, size_t N>
class ring_table {
    std::array<std::vector<T>, N> rows_;

public:
    explicit ring_table(size_t cols)
    {
        for (auto& row : rows_)
            row.resize(cols);
    }

    auto& operator[](size_t row) { return rows_[row % N]; }
};

}  // namespace step

#endif  // STEP_UTILITY_HPPs
