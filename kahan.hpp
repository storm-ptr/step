// Andrew Naplavkov

#ifndef STEP_KAHAN_HPP
#define STEP_KAHAN_HPP

#include <type_traits>
#include <utility>

namespace step {
namespace kahan {

/**
 * Reduces the numerical error in the total obtained by adding a sequence of
 * floating point numbers. (What about complex numbers?)
 * @see https://en.wikipedia.org/wiki/Kahan_summation_algorithm
 *
 * Supplied operations:
 * - less_than_comparable
 * - equality_comparable
 * - addable
 * - subtractable
 * @see
 * https://www.boost.org/doc/libs/1_67_0/libs/utility/operators.htm#arithmetic
 */
template <typename T>
class floating_point {
    static_assert(std::is_floating_point_v<T>);

    T value_{};
    T error_{};

    auto tie() const { return std::tie(value_, error_); }

public:
    /* implicit */ constexpr floating_point(const T& value) : value_{value} {}

    constexpr T value() const { return value_; }

    friend constexpr floating_point operator+(const floating_point& that)
    {
        return that;
    }

    friend constexpr floating_point operator-(const floating_point& that)
    {
        return {-that.value_, -that.error_};
    }

    constexpr floating_point& operator+=(const floating_point& that)
    {
        auto term = that.value_ - (this->error_ + that.error_);
        auto sum = this->value_ + term;
        this->error_ = (sum - this->value_) - term;
        this->value_ = sum;
        return *this;
    }

    constexpr floating_point& operator-=(const floating_point& that)
    {
        return *this += (-that);
    }

    friend constexpr bool operator==(const floating_point& lhs,
                                     const floating_point& rhs)
    {
        return lhs.tie() == rhs.tie();
    }

    friend constexpr bool operator<(const floating_point& lhs,
                                    const floating_point& rhs)
    {
        return lhs.tie() < rhs.tie();
    }
};

struct plus {
    template <typename Lhs, typename Rhs>
    constexpr auto operator()(const Lhs& lhs, const Rhs& rhs) const
    {
        return floating_point(lhs) += floating_point(rhs);
    }
};

struct minus {
    template <typename Lhs, typename Rhs>
    constexpr auto operator()(const Lhs& lhs, const Rhs& rhs) const
    {
        return floating_point(lhs) -= floating_point(rhs);
    }
};

}  // namespace kahan
}  // namespace step

#endif  // STEP_KAHAN_HPP
