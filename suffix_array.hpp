// Andrew Naplavkov

#ifndef STEP_SUFFIX_ARRAY_HPP
#define STEP_SUFFIX_ARRAY_HPP

#include <algorithm>
#include <step/detail/utility.hpp>

namespace step {

/**
 * Manber's algorithm for constructing suffix array.
 * Time complexity O(N*log(N)*log(N)), space complexity O(N),
 * where N is length of string.
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
    using sizes_type = std::vector<Size>;

    template <class InputIt>
    suffix_array(InputIt first, InputIt last)
        : str_(first, last), idx_(make_index())
    {
    }

    template <class InputRng>
    suffix_array(const InputRng& rng) : suffix_array(rng.begin(), rng.end())
    {
    }

    auto data() const { return str_.data(); }
    Size size() const { return str_.size(); }
    const auto& index() const { return idx_; }

    /**
     * Find position of an occurrence of the pattern as substring
     * in O(M*log(N)) time, where
     * M is length of the pattern, N is length of string
     */
    template <class InputIt>
    Size find(InputIt first, InputIt last) const
    {
        auto rng = equal_range(first, last);
        return rng.first == rng.second ? size() : *rng.first;
    }

    template <class InputRng>
    Size find(const InputRng& rng) const
    {
        return find(std::begin(rng), std::end(rng));
    }

    /// Find all occurrences of the pattern, @return pair of iterators.
    template <class InputIt>
    auto find_all(InputIt first, InputIt last) const
    {
        return equal_range(first, last);
    }

    template <class InputRng>
    auto find_all(const InputRng& rng) const
    {
        return find_all(std::begin(rng), std::end(rng));
    }

private:
    struct suffix {
        Size pos;
        std::pair<Size, Size> rank;
    };

    using suffixes_t = std::vector<suffix>;

    std::vector<T> str_;
    sizes_type idx_;

    template <class SuffixCompare>
    static void fill_first_rank(suffixes_t& sufs, const SuffixCompare& cmp)
    {
        auto it = sufs.begin();
        auto last = sufs.end();
        if (it == last)
            return;
        Size i{1};
        for (auto next = std::next(it); next != last; ++next, ++it) {
            bool less = cmp(*it, *next);
            it->rank.first = i;
            if (less)
                ++i;
        }
        it->rank.first = i;
    }

    static auto make_pos_to_rank_map(suffixes_t& sufs)
    {
        sizes_type result(sufs.size());
        for (Size i = 0; i < sufs.size(); ++i)
            result[sufs[i].pos] = sufs[i].rank.first;
        return result;
    }

    static void fill_second_rank(suffixes_t& sufs, Size offset)
    {
        auto map = make_pos_to_rank_map(sufs);
        for (auto& suf : sufs) {
            Size i = suf.pos + offset;
            suf.rank.second = i < map.size() ? map[i] : 0;
        }
    }

    static bool is_sorted(suffixes_t& sufs)
    {
        Size max_rank = sufs.empty() ? 0 : sufs.back().rank.first;
        return max_rank == sufs.size();
    }

    auto make_index() const
    {
        auto gen = [i = Size{}]() mutable { return suffix{i++}; };
        auto pos = [](auto& suf) { return suf.pos; };
        auto by_rank = [](auto& lhs, auto& rhs) { return lhs.rank < rhs.rank; };
        auto by_char = [=, cmp = Compare{}](auto& lhs, auto& rhs) {
            return cmp(str_[lhs.pos], str_[rhs.pos]);
        };

        suffixes_t sufs(size());
        std::generate(sufs.begin(), sufs.end(), gen);
        std::sort(sufs.begin(), sufs.end(), by_char);
        fill_first_rank(sufs, by_char);
        for (Size offset = 1; !is_sorted(sufs); offset *= 2) {
            fill_second_rank(sufs, offset);
            std::sort(sufs.begin(), sufs.end(), by_rank);
            fill_first_rank(sufs, by_rank);
        }

        sizes_type result(sufs.size());
        std::transform(sufs.begin(), sufs.end(), result.begin(), pos);
        return result;
    }

    auto compare_with(T val, Size offset) const
    {
        return [=, cmp = Compare{}](Size lhs, Size rhs) {
            bool lefty = lhs < size();
            Size pos = (lefty ? lhs : rhs) + offset;
            return pos >= size()
                       ? lefty
                       : lefty ? cmp(str_[pos], val) : cmp(val, str_[pos]);
        };
    }

    template <class InputIt>
    auto equal_range(InputIt first, InputIt last) const
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
};

template <class InputIt, class Size = size_t, class Compare = std::less<>>
suffix_array(InputIt, InputIt)
    ->suffix_array<iterator_value_t<InputIt>, Size, Compare>;

template <class InputRng, class Size = size_t, class Compare = std::less<>>
suffix_array(const InputRng&)
    ->suffix_array<range_value_t<InputRng>, Size, Compare>;

/**
 * Kasai's algorithm for constructing longest common prefix array.
 * @see https://en.wikipedia.org/wiki/LCP_array
 */
template <class T = char, class Size = size_t, class Compare = std::less<>>
class enhanced_suffix_array : public suffix_array<T, Size, Compare> {
public:
    template <class InputIt>
    enhanced_suffix_array(InputIt first, InputIt last)
        : suffix_array<T, Size, Compare>(first, last), lcp_(make_lcp())
    {
    }

    template <class InputRng>
    enhanced_suffix_array(const InputRng& rng)
        : enhanced_suffix_array(rng.begin(), rng.end())
    {
    }

    const auto& longest_common_prefix() const { return lcp_; }

private:
    sizes_type lcp_;

    auto iter(Size pos) const { return data() + pos; }
    auto iter() const { return iter(size()); }

    auto make_rank() const
    {
        sizes_type result(size());
        for (Size i = 0; i < size(); ++i)
            result[index()[i]] = i;
        return result;
    }

    auto make_lcp() const
    {
        auto result = sizes_type(size());
        auto rank = make_rank();
        auto eq = equivalence<Compare>{};
        for (Size i = 0, lcp = 0; i < size(); ++i) {
            auto cur = rank[i];
            auto next = cur + 1;
            lcp = next == size()
                      ? 0
                      : std::distance(iter(i),
                                      std::mismatch(iter(i + lcp),
                                                    iter(),
                                                    iter(index()[next] + lcp),
                                                    iter(),
                                                    eq)
                                          .first);
            result[cur] = lcp;
            if (lcp)
                --lcp;
        }
        return result;
    }
};

template <class InputIt, class Size = size_t, class Compare = std::less<>>
enhanced_suffix_array(InputIt, InputIt)
    ->enhanced_suffix_array<iterator_value_t<InputIt>, Size, Compare>;

template <class InputRng, class Size = size_t, class Compare = std::less<>>
enhanced_suffix_array(const InputRng&)
    ->enhanced_suffix_array<range_value_t<InputRng>, Size, Compare>;

}  // namespace step

#endif  // STEP_SUFFIX_ARRAY_HPP
