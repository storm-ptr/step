// Andrew Naplavkov

#include <fstream>
#include <iostream>
#include <step/edit_distance.hpp>
#include <string>
#include <string_view>

auto read_file(const char* file_name)
{
    using iterator_t = std::istreambuf_iterator<char>;
    return std::string{(iterator_t(std::ifstream{file_name})), iterator_t()};
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

int main(int argc, char* argv[])
{
    using option_t = std::optional<std::string_view>;
    auto file1 = read_file(argv[1]);
    auto file2 = read_file(argv[2]);
    auto rng1 = split(file1);
    auto rng2 = split(file2);
    std::vector<std::pair<option_t, option_t>> pairs;

    step::edit_distance::align(rng1, rng2, std::back_inserter(pairs));

    size_t row = 0;
    for (const auto& pair : pairs) {
        if (pair.second)
            ++row;
        if (pair.first != pair.second) {
            if (pair.first)
                std::cout << "--- " << row + (pair.second ? 0 : 1) << " ---\n"
                          << pair.first.value() << "\n";
            if (pair.second)
                std::cout << "+++ " << row << " +++\n"
                          << pair.second.value() << "\n";
        }
    }
}
