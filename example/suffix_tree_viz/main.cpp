// Andrew Naplavkov

#include <iostream>
#include <step/example/suffix_tree_viz/graphviz.hpp>

int main(int argc, char* argv[])
{
    std::cout << graphviz{std::string_view{argv[1]}};
}