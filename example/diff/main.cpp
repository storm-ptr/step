// Andrew Naplavkov

/**
 * @see https://en.wikipedia.org/wiki/Diff#Unified_format
 * @see
 * https://stackoverflow.com/questions/4045017/what-is-git-diff-patience-for
 */

#include <fstream>
#include <iostream>
#include <step/longest_common_subsequence.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

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

void print(view_iterator_t first1,
           view_iterator_t last1,
           view_iterator_t first2,
           view_iterator_t last2,
           size_t& offset1,
           size_t& offset2)
{
    size_t count1 = std::distance(first1, last1);
    size_t count2 = std::distance(first2, last2);
    if (!count1 && !count2)
        return;
    std::cout << "@@ -" << offset1 + 1 << "," << count1 << " +" << offset2 + 1
              << "," << count2 << " @@\n";
    std::for_each(
        first1, last1, [](auto view) { std::cout << "-" << view << "\n"; });
    std::for_each(
        first2, last2, [](auto view) { std::cout << "+" << view << "\n"; });
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
    views_t matches;
    step::longest_common_subsequence::intersection(
        first1, last1, first2, last2, std::back_inserter(matches));
    for (auto match : matches) {
        auto match1 = std::find(first1, last1, match);
        auto match2 = std::find(first2, last2, match);
        print(first1, match1, first2, match2, offset1, offset2);
        first1 = std::next(match1);
        first2 = std::next(match2);
        ++offset1;
        ++offset2;
    }
    print(first1, last1, first2, last2, offset1, offset2);
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

void patience_diff(view_iterator_t first1,
                   view_iterator_t last1,
                   view_iterator_t first2,
                   view_iterator_t last2,
                   size_t& offset1,
                   size_t& offset2)
{
    views_t marks;
    step::longest_common_subsequence::intersection(markers(first1, last1),
                                                   markers(first2, last2),
                                                   std::back_inserter(marks));
    for (auto mark : marks) {
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
