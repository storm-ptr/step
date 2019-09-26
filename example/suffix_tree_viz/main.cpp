// Andrew Naplavkov

#include <iostream>
#include <step/example/suffix_tree_viz/utility.hpp>

int main(int argc, char* argv[])
{
    std::string_view str{argv[1]};
    ordered_suffix_tree tree{};
    std::copy(str.begin(), str.end(), std::back_inserter(tree));
    std::cout << graphviz{tree};
}