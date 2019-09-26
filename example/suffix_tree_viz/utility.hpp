// Andrew Naplavkov

#ifndef STEP_EXAMPLE_SUFFIX_TREE_VIZ_UTILITY_HPP
#define STEP_EXAMPLE_SUFFIX_TREE_VIZ_UTILITY_HPP

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
            return os << "_" << me.edge.child;
    }
};

struct graphviz {
    ordered_suffix_tree& tree;

    friend std::ostream& operator<<(std::ostream& os, const graphviz& me)
    {
        os << "digraph \"" << std::string_view{me.tree.data(), me.tree.size()}
           << "\" {\nrankdir=LR\n";
        me.tree.visit([&](auto& edge) {
            if (edge.visited)
                return;
            os << child{me.tree, edge} << " [shape="
               << (me.tree.leaf(edge.child) ? "plaintext" : "point") << "]\n";
            if (!edge.child)
                return;
            auto rng = me.tree.substr(edge.child);
            os << "_" << edge.parent << "->" << child{me.tree, edge}
               << " [label=\""
               << std::string_view{me.tree.data() + rng.first,
                                   rng.second - rng.first}
               << "\"]\n";
        });
        return os << "}\n";
    }
};

#endif  // STEP_EXAMPLE_SUFFIX_TREE_VIZ_UTILITY_HPP
