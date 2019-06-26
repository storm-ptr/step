// Andrew Naplavkov

#ifndef STEP_KAHAN_HPP
#define STEP_KAHAN_HPP

#include <tuple>

namespace step::kahan {

/// Reduces the error in the total obtained by adding a floating-point sequence.

/// @see https://en.wikipedia.org/wiki/Kahan_summation_algorithm
template <class T>
class floating_point {
    T value_{};
    T error_{};

    constexpr auto tie() const { return std::tie(value_, error_); }

public:
    constexpr floating_point() = default;
    constexpr floating_point(const T& value) : value_{value} {}
    explicit operator T() const { return value_; }

    friend constexpr floating_point operator+(const floating_point& lhs,
                                              const floating_point& rhs)
    {
        auto term = rhs.value_ - (lhs.error_ + rhs.error_);
        auto result = floating_point(lhs.value_ + term);
        result.error_ = (result.value_ - lhs.value_) - term;
        return result;
    }

    friend constexpr floating_point operator-(const floating_point& that)
    {
        return {-that.value_, -that.error_};
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
    template <class Lhs, class Rhs>
    constexpr auto operator()(const Lhs& lhs, const Rhs& rhs) const
    {
        return floating_point(lhs) + floating_point(rhs);
    }
};

}  // namespace step::kahan

#endif  // STEP_KAHAN_HPP
