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
        : str_(first, last), idx_(make_index())
    {
    }

    template <class InputRng>
    suffix_array(const InputRng& rng) : suffix_array(rng.begin(), rng.end())
    {
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
        auto offset = Size{};
        std::for_each(first, last, [&](T val) {
            result = std::equal_range(result.first,
                                      result.second,
                                      size(),
                                      compare_with(val, offset++));
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
    auto make_longest_common_prefix_array() const
    {
        auto result = std::vector<Size>(size());
        auto inv = make_inverse_suffix_array();
        for (Size pos = 0, lcp = 0; pos < size(); ++pos) {
            auto cur = inv[pos];
            auto next = cur + 1;
            if (next < size()) {
                auto diff = std::mismatch(str_.begin() + pos + lcp,
                                          str_.end(),
                                          str_.begin() + idx_[next] + lcp,
                                          str_.end(),
                                          eq);
                lcp = std::distance(str_.begin() + pos, diff.first);
            }
            else
                lcp = 0;
            result[cur] = lcp;
            if (lcp)
                --lcp;
        }
        return result;
    }

private:
    inline static const auto cmp = Compare{};
    inline static const auto eq = equivalence<Compare>{};

    struct suffix {
        Size pos;
        std::pair<Size, Size> rank;
    };

    using suffixes = std::vector<suffix>;

    std::vector<T> str_;
    std::vector<Size> idx_;

    template <class SuffixCompare>
    static void fill_first_rank(suffixes& sufs, const SuffixCompare& suf_cmp)
    {
        auto it = sufs.begin();
        auto last = sufs.end();
        if (it == last)
            return;
        Size i{1};
        for (auto next = std::next(it); next != last; ++next, ++it) {
            bool less = suf_cmp(*it, *next);
            it->rank.first = i;
            if (less)
                ++i;
        }
        it->rank.first = i;
    }

    static auto make_inverse_first_rank(suffixes& sufs)
    {
        std::vector<Size> result(sufs.size());
        for (Size i = 0; i < sufs.size(); ++i)
            result[sufs[i].pos] = sufs[i].rank.first;
        return result;
    }

    static void fill_second_rank(suffixes& sufs, Size offset)
    {
        auto inv = make_inverse_first_rank(sufs);
        for (auto& suf : sufs) {
            Size pos = suf.pos + offset;
            suf.rank.second = pos < inv.size() ? inv[pos] : 0;
        }
    }

    static bool is_sorted(suffixes& sufs)
    {
        Size max_rank = sufs.empty() ? 0 : sufs.back().rank.first;
        return max_rank == sufs.size();
    }

    auto make_index() const
    {
        auto gen = [i = Size{}]() mutable { return suffix{i++}; };
        auto pos = [](auto& suf) { return suf.pos; };
        auto by_rank = [](auto& lhs, auto& rhs) { return lhs.rank < rhs.rank; };
        auto by_char = [this](auto& lhs, auto& rhs) {
            return cmp(str_[lhs.pos], str_[rhs.pos]);
        };

        suffixes sufs(size());
        std::generate(sufs.begin(), sufs.end(), gen);
        std::sort(sufs.begin(), sufs.end(), by_char);
        fill_first_rank(sufs, by_char);
        for (Size offset = 1; !is_sorted(sufs); offset *= 2) {
            fill_second_rank(sufs, offset);
            std::sort(sufs.begin(), sufs.end(), by_rank);
            fill_first_rank(sufs, by_rank);
        }

        std::vector<Size> result(sufs.size());
        std::transform(sufs.begin(), sufs.end(), result.begin(), pos);
        return result;
    }

    auto compare_with(T val, Size offset) const
    {
        return [=](Size lhs, Size rhs) {
            bool lefty = lhs < size();
            Size pos = (lefty ? lhs : rhs) + offset;
            return pos < size()
                       ? (lefty ? cmp(str_[pos], val) : cmp(val, str_[pos]))
                       : lefty;
        };
    }

    auto make_inverse_suffix_array() const
    {
        std::vector<Size> result(size());
        for (Size i = 0; i < size(); ++i)
            result[idx_[i]] = i;
        return result;
    }
};

template <class InputIt, class Size = size_t, class Compare = std::less<>>
suffix_array(InputIt, InputIt)
    ->suffix_array<iterator_value<InputIt>, Size, Compare>;

template <class InputRng, class Size = size_t, class Compare = std::less<>>
suffix_array(const InputRng&)
    ->suffix_array<range_value<InputRng>, Size, Compare>;

}  // namespace step

#endif  // STEP_SUFFIX_ARRAY_HPP
