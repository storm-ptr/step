// Andrew Naplavkov

#ifndef STEP_COMMON_HPP
#define STEP_COMMON_HPP

#include <array>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>

namespace step {

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

template <typename ForwardIt,
          typename T,
          typename OutputIt,
          typename Equal,
          typename BinaryOp>
auto associate_with_equal_or_tail(ForwardIt first,
                                  ForwardIt last,
                                  const T& val,
                                  OutputIt result,
                                  Equal equal,
                                  BinaryOp op)
{
    bool done = false;
    while (first != last) {
        auto next = std::next(first);
        if (!done && (equal(*first, val) || next == last)) {
            done = true;
            *result = op(*first, val);
        }
        else
            *result = op(*first, std::nullopt);
        ++result;
        first = next;
    }
    return result;
}

}  // namespace step

#endif  // STEP_COMMON_HPP
