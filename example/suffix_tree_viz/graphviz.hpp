// Andrew Naplavkov

#ifndef STEP_EXAMPLE_SUFFIX_TREE_VIZ_GRAPHVIZ_HPP
#define STEP_EXAMPLE_SUFFIX_TREE_VIZ_GRAPHVIZ_HPP

#include <map>
#include <ostream>
#include <step/suffix_tree.hpp>
#include <string_view>

template <class Key, class T>
using reverse_ordered_map = std::map<Key, T, std::greater<>>;

using ordered_suffix_tree =
    step::suffix_tree<char, size_t, reverse_ordered_map>;

struct child {
    ordered_suffix_tree& tree;
    ordered_suffix_tree::visited_edge edge;

    friend std::ostream& operator<<(std::ostream& os, const child& me)
    {
        if (me.tree.leaf(me.edge.child))
            return os << me.tree.path(me.edge).first;
        else
            return os << "node_" << me.edge.child;
    }
};

struct graphviz {
    std::string_view str;

    friend std::ostream& operator<<(std::ostream& os, const graphviz& me)
    {
        os << "digraph \"" << me.str << "\" {\n";
        ordered_suffix_tree tree{};
        std::copy(me.str.begin(), me.str.end(), std::back_inserter(tree));
        tree.visit([&](auto& edge) {
            if (edge.visited)
                return;
            os << child{tree, edge}
               << " [shape=" << (tree.leaf(edge.child) ? "plaintext" : "point")
               << "]\n";
            if (edge.child) {
                auto rng = tree.substr(edge.child);
                os << "node_" << edge.parent << "->" << child{tree, edge}
                   << " [label=\""
                   << std::string_view{tree.data() + rng.first,
                                       rng.second - rng.first}
                   << "\"]\n";
            }
        });
        tree.visit([&](auto& edge) {
            if (!edge.visited)
                return;
            if (auto link = tree.link(edge.child))
                os << child{tree, edge} << "->node_" << link
                   << " [style=dashed,arrowhead=otriangle]\n";
        });
        return os << "}\n";
    }
};

#endif  // STEP_EXAMPLE_SUFFIX_TREE_VIZ_GRAPHVIZ_HPP
