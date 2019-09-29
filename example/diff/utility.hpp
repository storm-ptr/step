// Andrew Naplavkov

#ifndef STEP_EXAMPLE_DIFF_UTILITY_HPP
#define STEP_EXAMPLE_DIFF_UTILITY_HPP

#include <ostream>
#include <step/longest_common_subsequence.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using lines_t = std::vector<std::string_view>;

inline lines_t split(const std::string& str)
{
    lines_t result;
    auto first = str.data();
    auto last = first + str.size();
    while (first != last) {
        auto it = std::find(first, last, '\n');
        result.emplace_back(first, std::distance(first, it));
        first = it == last ? it : std::next(it);
    }
    return result;
}

struct range {
    lines_t& lines;
    size_t first, last;

    auto begin() const { return lines.begin() + first; }
    auto end() const { return lines.begin() + last; }
    size_t size() const { return last - first; }
    size_t offset(lines_t::iterator it) const { return it - lines.begin(); }

    lines_t unique() const
    {
        lines_t result;
        std::unordered_map<std::string_view, size_t> counter;
        for (auto line : *this)
            ++counter[line];
        for (auto line : *this)
            if (counter.at(line) == 1)
                result.push_back(line);
        return result;
    }
};

template <class Consumer>
void join_subranges(range lhs,
                    range rhs,
                    const lines_t& separators,
                    Consumer consume)
{
    for (auto separator : separators) {
        auto l = std::find(lhs.begin(), lhs.end(), separator);
        auto r = std::find(rhs.begin(), rhs.end(), separator);
        consume({lhs.lines, lhs.first, lhs.offset(l)},
                {rhs.lines, rhs.first, rhs.offset(r)});
        lhs.first = lhs.offset(std::next(l));
        rhs.first = rhs.offset(std::next(r));
    }
    consume(lhs, rhs);
}

/// @see https://en.wikipedia.org/wiki/Diff#Unified_format
inline void print(range lhs, range rhs, std::ostream& os)
{
    if (!lhs.size() && !rhs.size())
        return;
    os << "@@ -" << lhs.offset(lhs.begin()) + 1 << "," << lhs.size() << " +"
       << rhs.offset(rhs.begin()) + 1 << "," << rhs.size() << " @@\n";
    for (auto line : lhs)
        os << "-" << line << "\n";
    for (auto line : rhs)
        os << "+" << line << "\n";
}

inline void diff(range lhs, range rhs, std::ostream& os)
{
    lines_t separators;
    step::longest_common_subsequence::intersection(
        lhs, rhs, std::back_inserter(separators));
    join_subranges(lhs, rhs, separators, [&os](range lhs, range rhs) {
        print(lhs, rhs, os);
    });
}

inline void patience_diff(range lhs, range rhs, std::ostream& os)
{
    lines_t separators;
    step::longest_common_subsequence::intersection(
        lhs.unique(), rhs.unique(), std::back_inserter(separators));
    join_subranges(lhs, rhs, separators, [&os](range lhs, range rhs) {
        diff(lhs, rhs, os);
    });
}

#endif  // STEP_EXAMPLE_DIFF_UTILITY_HPP
