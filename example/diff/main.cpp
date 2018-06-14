// Andrew Naplavkov

/// @see https://en.wikipedia.org/wiki/Diff#Unified_format

#include <fstream>
#include <iostream>
#include <step/longest_common_subsequence.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

using stream_iterator_t = std::istreambuf_iterator<char>;
using lines_t = std::vector<std::string_view>;
using line_iterator_t = lines_t::const_iterator;

auto read_file(const char* file_name)
{
    std::ifstream stream{file_name};
    return std::string{(stream_iterator_t(stream)), stream_iterator_t()};
}

auto split(const std::string& str)
{
    lines_t result;
    auto first = str.c_str();
    auto last = first + str.size();
    while (first != last) {
        auto it = std::find(first, last, '\n');
        result.emplace_back(first, std::distance(first, it));
        first = it == last ? it : std::next(it);
    }
    return result;
}

void print(line_iterator_t origin1,
           line_iterator_t first1,
           line_iterator_t last1,
           line_iterator_t origin2,
           line_iterator_t first2,
           line_iterator_t last2)
{
    size_t count1 = std::distance(first1, last1);
    size_t count2 = std::distance(first2, last2);
    if (count1 || count2) {
        std::cout << "@@ -" << std::distance(origin1, first1) + 1 << ","
                  << count1 << " +" << std::distance(origin2, first2) + 1 << ","
                  << count2 << " @@\n";
        std::for_each(
            first1, last1, [](auto line) { std::cout << "-" << line << "\n"; });
        std::for_each(
            first2, last2, [](auto line) { std::cout << "+" << line << "\n"; });
    }
}

void diff(line_iterator_t origin1,
          line_iterator_t first1,
          line_iterator_t last1,
          line_iterator_t origin2,
          line_iterator_t first2,
          line_iterator_t last2)
{
    lines_t matches;
    step::longest_common_subsequence::intersection(
        first1, last1, first2, last2, std::back_inserter(matches));
    for (auto match : matches) {
        auto match1 = std::find(first1, last1, match);
        auto match2 = std::find(first2, last2, match);
        print(origin1, first1, match1, origin2, first2, match2);
        first1 = std::next(match1);
        first2 = std::next(match2);
    }
    print(origin1, first1, last1, origin2, first2, last2);
}

lines_t markers(line_iterator_t first, line_iterator_t last)
{
    lines_t result;
    std::unordered_map<std::string_view, size_t> counter;
    std::for_each(first, last, [&](auto line) { ++counter[line]; });
    std::copy_if(first, last, std::back_inserter(result), [&](auto line) {
        return counter[line] == 1;
    });
    return result;
}

void patience_diff(line_iterator_t origin1,
                   line_iterator_t first1,
                   line_iterator_t last1,
                   line_iterator_t origin2,
                   line_iterator_t first2,
                   line_iterator_t last2)
{
    lines_t matches;
    step::longest_common_subsequence::intersection(markers(first1, last1),
                                                   markers(first2, last2),
                                                   std::back_inserter(matches));
    for (auto match : matches) {
        auto match1 = std::find(first1, last1, match);
        auto match2 = std::find(first2, last2, match);
        diff(origin1, first1, match1, origin2, first2, match2);
        first1 = std::next(match1);
        first2 = std::next(match2);
    }
    diff(origin1, first1, last1, origin2, first2, last2);
}

int main(int argc, char* argv[])
{
    std::cout << "--- " << argv[1] << "\n+++ " << argv[2] << "\n";
    auto file1 = read_file(argv[1]);
    auto file2 = read_file(argv[2]);
    auto lines1 = split(file1);
    auto lines2 = split(file2);
    patience_diff(lines1.cbegin(),
                  lines1.cbegin(),
                  lines1.cend(),
                  lines2.cbegin(),
                  lines2.cbegin(),
                  lines2.cend());
}
