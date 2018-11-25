// Andrew Naplavkov

#ifndef STEP_SUFFIX_ARRAY_HPP
#define STEP_SUFFIX_ARRAY_HPP

#include <algorithm>
#include <step/detail/utility.hpp>

namespace step {

/**
 * Manber's algorithm for constructing suffix array.
 * Time complexity O(N*log(N)*log(N)), space complexity O(N),
 * where N is length of text.
 * @param T - type of the characters;
 * @param Size - to specify the maximum number of characters;
 * @param Compare - to determine the order of characters.
 * @see https://en.wikipedia.org/wiki/Suffix_array
 */
template <class T = char, class Size = size_t, class Compare = std::less<>>
class suffix_array {
public:
    using value_type = T;
    using size_type = Size;

    Size size() const { return str_.size(); }
    auto data() const { return str_.data(); }
    Size nth_element(Size nth) const { return idx_[nth]; }

    template <class InputIt>
    suffix_array(InputIt first, InputIt last)
        : suffix_array(std::vector<T>(first, last))
    {
    }

    template <class InputRng>
    suffix_array(const InputRng& rng)
        : suffix_array(std::begin(rng), std::end(rng))
    {
    }

    suffix_array(std::vector<T>&& str) : str_(std::move(str)), idx_(size())
    {
        auto gen = [i = Size{}]() mutable { return suffix{i++}; };
        auto pos = [](auto& sfx) { return sfx.pos; };
        auto by_rank = [](auto& lhs, auto& rhs) { return lhs.rank < rhs.rank; };
        auto by_char = [this](auto& lhs, auto& rhs) {
            return cmp_(str_[lhs.pos], str_[rhs.pos]);
        };

        std::vector<suffix> sfxs(size());
        std::generate(sfxs.begin(), sfxs.end(), gen);
        std::sort(sfxs.begin(), sfxs.end(), by_char);
        fill_first_rank(sfxs.begin(), sfxs.end(), by_char);
        for (Size shift = 1; !is_sorted(sfxs.begin(), sfxs.end()); shift *= 2) {
            fill_second_rank(sfxs.begin(), sfxs.end(), shift);
            std::sort(sfxs.begin(), sfxs.end(), by_rank);
            fill_first_rank(sfxs.begin(), sfxs.end(), by_rank);
        }
        std::transform(sfxs.begin(), sfxs.end(), idx_.begin(), pos);
    }

    /**
     * Find all occurrences of the pattern as substring in O(M*log(N)) time,
     * where M is length of the pattern, N is length of text.
     * @return pair of iterators.
     */
    template <class InputIt>
    auto find_all(InputIt first, InputIt last) const
    {
        auto result = std::make_pair(idx_.begin(), idx_.end());
        auto shift = Size{};
        std::for_each(first, last, [&](T val) {
            result = std::equal_range(result.first,
                                      result.second,
                                      size(),
                                      compare_with(val, shift++));
        });
        return result;
    }

    template <class InputRng>
    auto find_all(const InputRng& rng) const
    {
        return find_all(std::begin(rng), std::end(rng));
    }

    /// @return position of the pattern as substring
    template <class InputIt>
    Size find(InputIt first, InputIt last) const
    {
        auto rng = find_all(first, last);
        return rng.first == rng.second ? size() : *rng.first;
    }

    template <class InputRng>
    Size find(const InputRng& rng) const
    {
        return find(std::begin(rng), std::end(rng));
    }

    /**
     * Kasai's algorithm for constructing longest common prefix array.
     * Time and space complexity O(N), where N is length of text.
     * @see https://en.wikipedia.org/wiki/LCP_array
     */
    template <class RandomIt>
    void longest_common_prefix_array(RandomIt result) const
    {
        std::vector<Size> inverse(size());
        for (Size i = 0; i < size(); ++i)
            inverse[idx_[i]] = i;
        for (Size pos = 0, lcp = 0; pos < size(); ++pos) {
            auto cur = inverse[pos];
            auto next = cur + 1;
            if (next < size()) {
                auto diff = std::mismatch(str_.begin() + pos + lcp,
                                          str_.end(),
                                          str_.begin() + idx_[next] + lcp,
                                          str_.end(),
                                          eq_);
                lcp = std::distance(str_.begin() + pos, diff.first);
            }
            else
                lcp = 0;
            result[cur] = lcp;
            if (lcp)
                --lcp;
        }
    }

private:
    inline static const auto cmp_ = Compare{};
    inline static const auto eq_ = equivalence<Compare>{};

    struct suffix {
        Size pos;
        std::pair<Size, Size> rank;
    };

    std::vector<T> str_;
    std::vector<Size> idx_;

    template <class SuffixIt, class Cmp>
    static void fill_first_rank(SuffixIt first, SuffixIt last, const Cmp& cmp)
    {
        if (first == last)
            return;
        Size i{1};
        for (auto next = std::next(first); next != last; ++next, ++first) {
            bool less = cmp(*first, *next);
            first->rank.first = i;
            if (less)
                ++i;
        }
        first->rank.first = i;
    }

    template <class SuffixIt>
    static void fill_second_rank(SuffixIt first, SuffixIt last, Size shift)
    {
        std::vector<Size> inverse(std::distance(first, last));
        std::for_each(
            first, last, [&](auto& sfx) { inverse[sfx.pos] = sfx.rank.first; });
        std::for_each(first, last, [&](auto& sfx) {
            Size pos = sfx.pos + shift;
            sfx.rank.second = pos < inverse.size() ? inverse[pos] : 0;
        });
    }

    template <class SuffixIt>
    static bool is_sorted(SuffixIt first, SuffixIt last)
    {
        return first == last ||
               std::prev(last)->rank.first == std::distance(first, last);
    }

    auto compare_with(T val, Size shift) const
    {
        return [=](Size lhs, Size rhs) {
            bool lefty = lhs < size();
            Size pos = (lefty ? lhs : rhs) + shift;
            return pos < size()
                       ? (lefty ? cmp_(str_[pos], val) : cmp_(val, str_[pos]))
                       : lefty;
        };
    }
};

template <class InputIt>
suffix_array(InputIt, InputIt)->suffix_array<iterator_value<InputIt>>;

template <class InputRng>
suffix_array(InputRng)->suffix_array<range_value<InputRng>>;

}  // namespace step

#endif  // STEP_SUFFIX_ARRAY_HPP
