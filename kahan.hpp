// Andrew Naplavkov

#ifndef STEP_KAHAN_HPP
#define STEP_KAHAN_HPP

#include <tuple>

namespace step {
namespace kahan {

/**
 * Reduces the numerical error in the total obtained by adding a sequence of
 * floating point numbers. (What about complex numbers?)
 * @see https://en.wikipedia.org/wiki/Kahan_summation_algorithm
 */
template <typename T>
class floating_point {
    T value_{};
    T error_{};

    auto tie() const { return std::tie(value_, error_); }

public:
    /* implicit */ constexpr floating_point(const T& value) : value_{value} {}

    constexpr T value() const { return value_; }

    friend constexpr floating_point operator+(const floating_point& lhs,
                                              const floating_point& rhs)
    {
        auto term = rhs.value_ - (lhs.error_ + rhs.error_);
        floating_point result(lhs.value_ + term);
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
    template <typename Lhs, typename Rhs>
    constexpr auto operator()(const Lhs& lhs, const Rhs& rhs) const
    {
        return floating_point(lhs) + floating_point(rhs);
    }
};

}  // namespace kahan
}  // namespace step

#endif  // STEP_KAHAN_HPP
