// Andrew Naplavkov

#include <fstream>
#include <iomanip>
#include <iostream>
#include <step/edit_distance.hpp>
#include <string>
#include <string_view>

auto read_file(const char* file_name)
{
    using iterator_t = std::istreambuf_iterator<char>;
    std::ifstream stream{file_name};
    return std::string{(iterator_t(stream)), iterator_t()};
}

auto split(const std::string& str)
{
    std::vector<std::string_view> result;
    auto first = str.c_str();
    auto last = first + str.size();
    while (first != last) {
        auto it = std::find(first, last, '\n');
        result.emplace_back(first, std::distance(first, it));
        first = it == last ? it : std::next(it);
    }
    return result;
}

using option_t = std::optional<std::string_view>;
using pairs_t = std::vector<std::pair<option_t, option_t>>;

void print_diff(const pairs_t& pairs)
{
    size_t row1 = 1;
    size_t row2 = 1;
    for (auto it = pairs.begin(); it != pairs.end();) {
        auto first = std::find_if(it, pairs.end(), [](const auto& pair) {
            return pair.first != pair.second;
        });
        auto last = std::find_if(first, pairs.end(), [](const auto& pair) {
            return pair.first == pair.second;
        });
        row1 += std::distance(it, first);
        row2 += std::distance(it, first);
        if (first != last)
            std::cout << row1 << "," << row2 << "\n";
        std::for_each(first, last, [&row1](const auto& pair) {
            if (pair.first) {
                std::cout << "- " << pair.first.value() << "\n";
                ++row1;
            }
        });
        std::for_each(first, last, [&row2](const auto& pair) {
            if (pair.second) {
                std::cout << "+ " << pair.second.value() << "\n";
                ++row2;
            }
        });
        it = last;
    }
}

int main(int argc, char* argv[])
{
    auto file1 = read_file(argv[1]);
    auto file2 = read_file(argv[2]);
    pairs_t pairs;
    step::edit_distance::associate(
        split(file1), split(file2), std::back_inserter(pairs));
    print_diff(pairs);
}
