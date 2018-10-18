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
    using sizes_t = std::vector<Size>;

    std::vector<T> str_;
    sizes_t idx_;

    template <class Predicate>
    static void fill_first_rank(suffixes_t& sufs, const Predicate& pred)
    {
        auto it = sufs.begin();
        auto last = sufs.end();
        for (Size rank = 1; it != last; ++rank) {
            auto next = std::adjacent_find(it, last, pred);
            if (next != last)
                ++next;
            std::for_each(it, next, [=](auto& suf) { suf.rank.first = rank; });
            it = next;
        }
    }

    static auto make_pos_to_rank_map(const suffixes_t& sufs)
    {
        sizes_t result(sufs.size());
        for (Size i = 0; i < sufs.size(); ++i)
            result[sufs[i].pos] = sufs[i].rank.first;
        return result;
    }

    static void fill_second_rank(suffixes_t& sufs, Size offset)
    {
        auto map = make_pos_to_rank_map(sufs);
        for (auto& suf : sufs) {
            Size pos = suf.pos + offset;
            suf.rank.second = pos < map.size() ? map[pos] : 0;
        }
    }

    static bool is_sorted(const suffixes_t& sufs)
    {
        Size max_rank = sufs.empty() ? 0 : sufs.back().rank.first;
        return max_rank == sufs.size();
    }

    auto make_index() const
    {
        auto generator = [pos = Size{}]() mutable { return suffix{pos++}; };
        auto pos = [](auto& suf) { return suf.pos; };
        auto by_rank = [](auto& lhs, auto& rhs) { return lhs.rank < rhs.rank; };
        auto by_char = [this, cmp = Compare{}](auto& lhs, auto& rhs) {
            return cmp(str_[lhs.pos], str_[rhs.pos]);
        };

        suffixes_t sufs(size());
        std::generate(sufs.begin(), sufs.end(), generator);
        std::sort(sufs.begin(), sufs.end(), by_char);
        fill_first_rank(sufs, by_char);
        for (Size offset = 1; !is_sorted(sufs); offset *= 2) {
            fill_second_rank(sufs, offset);
            std::sort(sufs.begin(), sufs.end(), by_rank);
            fill_first_rank(sufs, by_rank);
        }

        sizes_t result(sufs.size());
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
        Size offset{};
        std::for_each(first, last, [&](T val) {
            result = std::equal_range(result.first,
                                      result.second,
                                      size(),
                                      compare_with(val, offset++));
        });
        return result;
    }
};

template <class InputIt,
          class Policy = std::execution::sequenced_policy,
          class Size = size_t,
          class Compare = std::less<>>
suffix_array(InputIt, InputIt, const Policy& = Policy())
    ->suffix_array<iterator_value_t<InputIt>, Size, Compare>;

template <class InputRng,
          class Policy = std::execution::sequenced_policy,
          class Size = size_t,
          class Compare = std::less<>>
suffix_array(const InputRng&, const Policy& = Policy())
    ->suffix_array<range_value_t<InputRng>, Size, Compare>;

}  // namespace step

#endif  // STEP_SUFFIX_ARRAY_HPP
