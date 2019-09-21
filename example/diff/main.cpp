// Andrew Naplavkov

#include <fstream>
#include <iostream>
#include <step/example/diff/patience_diff.hpp>
#include <string>

std::string read_file(const char* file_name)
{
    using iterator_t = std::istreambuf_iterator<char>;
    std::ifstream is{file_name};
    return {(iterator_t(is)), iterator_t()};
}

int main(int argc, char* argv[])
{
    std::cout << "--- " << argv[1] << "\n+++ " << argv[2] << "\n";
    auto file1 = read_file(argv[1]);
    auto file2 = read_file(argv[2]);
    auto lines1 = split(file1);
    auto lines2 = split(file2);
    patience_diff(
        {lines1, 0, lines1.size()}, {lines2, 0, lines2.size()}, std::cout);
}
