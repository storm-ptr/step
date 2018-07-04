// Andrew Naplavkov

#ifndef STEP_COMMON_HPP
#define STEP_COMMON_HPP

#include <algorithm>
#include <array>
#include <iterator>
#include <numeric>
#include <utility>
#include <vector>

namespace step {

struct min {
    template <typename T>
    const T& operator()(const T& lhs, const T& rhs) const
    {
        return std::min(lhs, rhs);
    }
};

struct max {
    template <typename T>
    const T& operator()(const T& lhs, const T& rhs) const
    {
        return std::max(lhs, rhs);
    }
};

struct gcd {
    template <typename Lhs, typename Rhs>
    auto operator()(Lhs lhs, Rhs rhs) const
    {
        return std::gcd(lhs, rhs);
    }
};

struct make_pair {
    template <typename Lhs, typename Rhs>
    auto operator()(Lhs&& lhs, Rhs&& rhs)
    {
        return std::make_pair(std::forward<Lhs>(lhs), std::forward<Rhs>(rhs));
    }
};

struct make_reverse_pair {
    template <typename Lhs, typename Rhs>
    auto operator()(Lhs&& lhs, Rhs&& rhs)
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

#endif  // STEP_COMMON_HPP
