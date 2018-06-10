// Andrew Naplavkov

/**
 * @see https://en.wikipedia.org/wiki/Diff#Unified_format
 * @see
 * https://stackoverflow.com/questions/4045017/what-is-git-diff-patience-for
 */

#include <fstream>
#include <iostream>
#include <step/edit_distance.hpp>
#include <step/longest_common_subsequence.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

using option_t = std::optional<std::string_view>;
using pairs_t = std::vector<std::pair<option_t, option_t>>;
using pair_iterator_t = pairs_t::const_iterator;
using stream_iterator_t = std::istreambuf_iterator<char>;
using views_t = std::vector<std::string_view>;
using view_iterator_t = views_t::const_iterator;

auto read_file(const char* file_name)
{
    std::ifstream stream{file_name};
    return std::string{(stream_iterator_t(stream)), stream_iterator_t()};
}

auto split(const std::string& str)
{
    views_t result;
    auto first = str.c_str();
    auto last = first + str.size();
    while (first != last) {
        auto it = std::find(first, last, '\n');
        result.emplace_back(first, std::distance(first, it));
        first = it == last ? it : std::next(it);
    }
    return result;
}

auto find_change_hunk(pair_iterator_t first, pair_iterator_t last)
{
    auto result_first = std::find_if(first, last, [](const auto& pair) {
        return pair.first != pair.second;
    });
    auto result_last = std::find_if(result_first, last, [](const auto& pair) {
        return pair.first == pair.second;
    });
    return std::make_pair(result_first, result_last);
}

void print_change_hunk(pair_iterator_t first,
                       pair_iterator_t last,
                       size_t& offset1,
                       size_t& offset2)
{
    size_t count1 = std::count_if(
        first, last, [](const auto& pair) { return !!pair.first; });
    size_t count2 = std::count_if(
        first, last, [](const auto& pair) { return !!pair.second; });
    std::cout << "@@ -" << offset1 + 1 << "," << count1 << " +" << offset2 + 1
              << "," << count2 << " @@\n";
    std::for_each(first, last, [](const auto& pair) {
        if (pair.first)
            std::cout << "-" << pair.first.value() << "\n";
    });
    std::for_each(first, last, [](const auto& pair) {
        if (pair.second)
            std::cout << "+" << pair.second.value() << "\n";
    });
    offset1 += count1;
    offset2 += count2;
}

void diff(view_iterator_t first1,
          view_iterator_t last1,
          view_iterator_t first2,
          view_iterator_t last2,
          size_t& offset1,
          size_t& offset2)
{
    pairs_t pairs;
    step::edit_distance::associate(
        first1, last1, first2, last2, std::back_inserter(pairs));
    for (auto it = pairs.cbegin(); it != pairs.cend();) {
        auto[first, last] = find_change_hunk(it, pairs.cend());
        offset1 += std::distance(it, first);
        offset2 += std::distance(it, first);
        if (first != last)
            print_change_hunk(first, last, offset1, offset2);
        it = last;
    }
}

views_t markers(view_iterator_t first, view_iterator_t last)
{
    views_t result;
    std::unordered_map<std::string_view, size_t> counter;
    std::for_each(first, last, [&](auto item) { ++counter[item]; });
    std::copy_if(first, last, std::back_inserter(result), [&](auto item) {
        return counter[item] == 1;
    });
    return result;
}

views_t markers(view_iterator_t first1,
                view_iterator_t last1,
                view_iterator_t first2,
                view_iterator_t last2)
{
    views_t result;
    step::longest_common_subsequence::intersection(markers(first1, last1),
                                                   markers(first2, last2),
                                                   std::back_inserter(result));
    return result;
}

void patience_diff(view_iterator_t first1,
                   view_iterator_t last1,
                   view_iterator_t first2,
                   view_iterator_t last2,
                   size_t& offset1,
                   size_t& offset2)
{
    for (auto mark : markers(first1, last1, first2, last2)) {
        auto mark1 = std::find(first1, last1, mark);
        auto mark2 = std::find(first2, last2, mark);
        diff(first1, mark1, first2, mark2, offset1, offset2);
        first1 = std::next(mark1);
        first2 = std::next(mark2);
        ++offset1;
        ++offset2;
    }
    diff(first1, last1, first2, last2, offset1, offset2);
}

int main(int argc, char* argv[])
{
    std::cout << "--- " << argv[1] << "\n+++ " << argv[2] << "\n";
    auto file1 = read_file(argv[1]);
    auto file2 = read_file(argv[2]);
    auto views1 = split(file1);
    auto views2 = split(file2);
    size_t offset1 = 0;
    size_t offset2 = 0;
    patience_diff(views1.cbegin(),
                  views1.cend(),
                  views2.cbegin(),
                  views2.cend(),
                  offset1,
                  offset2);
}
