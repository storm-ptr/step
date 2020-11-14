// Andrew Naplavkov

#ifndef STEP_SUFFIX_TREE_HPP
#define STEP_SUFFIX_TREE_HPP

#include "detail/utility.hpp"
#include <optional>
#include <stack>
#include <unordered_map>

namespace step {

/// Ukkonen's online algorithm for constructing suffix tree.

/// Time complexity O(N*log(K)), space complexity O(N), where:
/// N - text length, K - alphabet size.
/// @param T - type of the characters;
/// @param Size - to specify the maximum number / offset of characters;
/// @param Map - to associate characters with edges, its key_type shall be T.
///              std::map is preferable for small alphabet.
/// @see https://en.wikipedia.org/wiki/Suffix_tree
template <class T = char,
          class Size = size_t,
          template <class...> class Map = std::unordered_map>
class suffix_tree {
public:
    using value_type = T;
    using size_type = Size;
    using substring = std::pair<Size, Size>;  ///< half-open offset range

    auto data() const { return str_.data(); }
    Size size() const { return (Size)str_.size(); }

    void clear() noexcept
    {
        str_.clear();
        nodes_.clear();
        char_ = node_ = 0;
    }

    void reserve(Size len)
    {
        str_.reserve(len);
        nodes_.reserve(len);
    }

    /// Basic exception guarantee
    void push_back(T val)
    try {
        str_.push_back(val);
        if (nodes_.empty())
            nodes_.emplace_back();
        auto tie = [&, src = nodes()](Size dest) mutable {
            if (!leaf(src) && src != dest)
                nodes_[src++].link = dest;
        };
        while (reminder()) {
            if (Size& child = nodes_[node_].children[str_[char_]]) {
                if (descend(child))
                    continue;
                if (!split(child))
                    return tie(node_);
                tie(nodes() - 1);
            }
            else {
                child = flip(char_);
                tie(node_);
            }
            node_ ? node_ = nodes_[node_].link : ++char_;
        }
    }
    catch (...) {
        clear();
        throw;
    }

    /// Find offset of the first occurrence of the substring.

    /// Time complexity O(M), where: M - substring length.
    template <class InputIt>
    Size find(InputIt first, InputIt last) const
    {
        auto edge = find_edge(first, last);
        return edge ? path(*edge).first : size();
    }

    template <class InputRng>
    Size find(const InputRng& rng) const
    {
        return find(std::begin(rng), std::end(rng));
    }

    /// Find all occurrences of the substring for explicit suffix tree.

    /// Explicit means padded with a terminal symbol not seen in the text.
    template <class InputIt, class OutputIt>
    OutputIt find_all(InputIt first, InputIt last, OutputIt result) const
    {
        if (auto src = find_edge(first, last))
            dfs(*src, [&](auto& edge) {
                if (leaf(edge.child))
                    *result++ = path(edge).first;
            });
        return result;
    }

    template <class InputRng, class OutputIt>
    OutputIt find_all(const InputRng& rng, OutputIt result) const
    {
        return find_all(std::begin(rng), std::end(rng), result);
    }

    /// Callback parameter
    struct visited_edge {
        Size parent;   ///< parent node
        Size child;    ///< child node
        Size path;     ///< number of characters from the root
        bool visited;  ///< pre/post-order
    };

    /// Depth-first tree traversal.

    /// @code tree.visit([](const visited_edge&) {}); @endcode
    template <class Visitor>
    void visit(Visitor&& viz) const
    {
        if (!nodes_.empty())
            dfs({}, std::forward<Visitor>(viz));
    }

    bool leaf(Size node) const { return node >= nodes(); }

    substring substr(Size node) const
    {
        return leaf(node) ? substring{flip(node), size()} : nodes_[node].rng;
    }

    substring path(const visited_edge& edge) const
    {
        Size last = substr(edge.child).second;
        return {Size(last - edge.path), last};
    }

private:
    inline static const auto eq_ = key_equal_or_equivalence_t<Map<T, Size>>{};

    struct inner_node {
        Map<T, Size> children;
        substring rng;
        Size link;
    };

    std::vector<T> str_;
    std::vector<inner_node> nodes_;
    Size char_{}, node_{};  // active

    Size reminder() const { return size() - char_; }
    Size nodes() const { return (Size)nodes_.size(); }

    bool descend(Size node)
    {
        Size len = step::size(substr(node));
        if (reminder() <= len)
            return false;
        char_ += len;
        node_ = node;
        return true;
    }

    bool split(Size& child)
    {
        auto rng = substr(child);
        Size cut = rng.first + reminder() - 1;
        Size back = size() - 1;
        if (eq_(str_[cut], str_[back]))
            return false;
        Size old = std::exchange(child, nodes());
        nodes_.push_back({{{str_[cut], leaf(old) ? flip(cut) : old},
                           {str_[back], flip(back)}},
                          {rng.first, cut}});
        if (!leaf(old))
            nodes_[old].rng = {cut, rng.second};
        return true;
    }

    template <class InputIt>
    std::optional<visited_edge> find_edge(InputIt first, InputIt last) const
    {
        for (visited_edge edge{}; !nodes_.empty();) {
            auto rng = substr(edge.child);
            edge.path += step::size(rng);
            auto diff = std::mismatch(
                first, last, data() + rng.first, data() + rng.second, eq_);
            if (diff.first == last)
                return edge;
            if (diff.second != data() + rng.second || leaf(edge.child))
                break;
            auto it = nodes_[edge.child].children.find(*diff.first);
            if (it == nodes_[edge.child].children.end())
                break;
            first = diff.first;
            edge.parent = std::exchange(edge.child, it->second);
        }
        return std::nullopt;
    }

    void spawn(visited_edge src, std::stack<visited_edge>& dest) const
    {
        for (auto& pair : nodes_[src.child].children)
            dest.push({src.child,
                       pair.second,
                       Size(src.path + step::size(substr(pair.second)))});
    }

    template <class Visitor>
    void dfs(const visited_edge& src, Visitor viz) const
    {
        for (std::stack<visited_edge> stack{{src}}; !stack.empty();) {
            auto& top = stack.top();
            viz(std::as_const(top));
            if (leaf(top.child) || std::exchange(top.visited, true))
                stack.pop();
            else
                spawn(top, stack);
        }
    }
};

}  // namespace step

#endif  // STEP_SUFFIX_TREE_HPP
