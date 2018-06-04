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

class output_iterator {
    size_t row_ = 0;

public:
    using iterator_category = std::output_iterator_tag;
    using option_t = std::optional<std::string_view>;
    output_iterator& operator*() { return *this; }
    output_iterator& operator++() { return *this; }
    output_iterator& operator=(const std::pair<option_t, option_t>& pair)
    {
        if (pair.second)
            ++row_;
        if (pair.first != pair.second) {
            if (pair.first)
                std::cout << std::setw(4) << std::right
                          << row_ + (pair.second ? 0 : 1) << " << "
                          << pair.first.value() << "\n";
            if (pair.second)
                std::cout << std::setw(4) << std::right << row_ << " >> "
                          << pair.second.value() << "\n";
        }
        return *this;
    }
};

int main(int argc, char* argv[])
{
    auto file1 = read_file(argv[1]);
    auto file2 = read_file(argv[2]);
    step::edit_distance::align(split(file1), split(file2), output_iterator{});
}
