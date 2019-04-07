// Andrew Naplavkov

#ifndef STEP_SUFFIX_TREE_HPP
#define STEP_SUFFIX_TREE_HPP

#include <optional>
#include <stack>
#include <step/detail/utility.hpp>
#include <unordered_map>

namespace step {

/**
 * Ukkonen's online algorithm for constructing suffix tree.
 * Time complexity O(N*log(N)), space complexity O(N), where:
 * N is length of text.
 * @param T - type of the characters;
 * @param Size - to specify the maximum number of characters;
 * @param Map - to associate characters with edges, its key_type shall be T.
 * @see https://en.wikipedia.org/wiki/Suffix_tree
 */
template <class T = char,
          class Size = size_t,
          template <class...> class Map = std::unordered_map>
class suffix_tree {
public:
    using value_type = T;
    using size_type = Size;
    using substring = std::pair<Size, Size>;  // half-open position range

    struct visited_edge {
        Size parent;
        Size child;
        Size path_len;  // number of characters from the root to the child node
        bool visited;   // pre/post-order traversal
    };

    auto size() const { return (Size)str_.size(); }
    auto data() const { return str_.data(); }
    auto begin(const substring& str) const { return data() + str.first; }
    auto end(const substring& str) const { return data() + str.second; }
    bool leaf(Size node) const { return node >= nodes(); }

    substring substr(Size node) const
    {
        return leaf(node) ? substring{flip(node), size()} : nodes_[node].str;
    }

    substring path(const visited_edge& edge) const
    {
        auto str = substr(edge.child);
        return substring{Size(str.second - edge.path_len), str.second};
    }

    void clear() noexcept
    {
        str_.clear();
        nodes_.clear();
        char_ = 0;
        node_ = 0;
    }

    void reserve(Size len)
    {
        str_.reserve(len);
        nodes_.reserve(len);
    }

    /// Basic exception guarantee.
    void push_back(T val) try {
        str_.push_back(val);
        for (auto connect = connector(); reminder();) {
            if (auto& child = nodes_[node_].children[str_[char_]]) {
                if (descend(child))
                    continue;
                if (!split(child))
                    return connect(node_);
                connect(nodes() - 1);
            }
            else {
                child = flip(char_);
                connect(node_);
            }
            if (node_)  // not root
                node_ = nodes_[node_].link;
            else
                ++char_;
        }
    }
    catch (...) {
        clear();
        throw;
    }

    /**
     * Find position of the first occurrence of the pattern as substring
     * in O(M) time, where M is length of the pattern.
     */
    template <class InputIt>
    Size find(InputIt first, InputIt last) const
    {
        auto origin = find_edge(first, last);
        return origin ? path(*origin).first : size();
    }

    template <class InputRng>
    Size find(const InputRng& rng) const
    {
        return find(std::begin(rng), std::end(rng));
    }

    /**
     * Find all occurrences of the pattern for explicit suffix tree
     * (padded with a terminal symbol not seen in the string).
     */
    template <class InputIt, class OutputIt>
    OutputIt find_all(InputIt first, InputIt last, OutputIt result) const
    {
        if (auto origin = find_edge(first, last))
            dfs(*origin, [&](const auto& edge) {
                if (!edge.visited && leaf(edge.child))
                    *result++ = path(edge).first;
            });
        return result;
    }

    template <class InputRng, class OutputIt>
    OutputIt find_all(const InputRng& rng, OutputIt result) const
    {
        return find_all(std::begin(rng), std::end(rng), result);
    }

    /**
     * Depth-first traversal of the tree.
     * Pre/post-order @param visitor has @param visited_edge.
     */
    template <class Visitor>
    void visit(Visitor&& visitor) const
    {
        if (!nodes_.empty())
            dfs({}, std::forward<Visitor>(visitor));
    }

private:
    inline static const auto eq_ = key_equal_or_equivalence_t<Map<T, Size>>{};

    struct internal_node {
        Map<T, Size> children;
        substring str;
        Size link;
    };

    std::vector<T> str_;
    std::vector<internal_node> nodes_;
    Size char_ = 0;  // active edge character
    Size node_ = 0;  // active node

    Size reminder() const { return size() - char_; }
    auto nodes() const { return (Size)nodes_.size(); }

    auto connector()
    {
        if (nodes_.empty())
            nodes_.emplace_back();  // root
        return [this, last = nodes()](Size node) mutable {
            if (last < nodes() && last != node)
                nodes_[last++].link = node;
        };
    }

    bool descend(Size node)
    {
        auto len = step::size(substr(node));
        if (reminder() <= len)
            return false;
        char_ += len;
        node_ = node;
        return true;
    }

    bool split(Size& child)
    {
        auto str = substr(child);
        auto head = substring{str.first, Size(str.first + reminder() - 1)};
        auto tail = substring{head.second, str.second};
        if (eq_(str_[tail.first], str_.back()))
            return false;
        auto old = std::exchange(child, nodes());
        nodes_.push_back({{{str_.back(), flip<Size>(size() - 1)}}, head});
        if (leaf(old))
            nodes_.back().children[str_[tail.first]] = flip(tail.first);
        else {
            nodes_.back().children[str_[tail.first]] = old;
            nodes_[old].str = tail;
        }
        return true;
    }

    template <class InputIt>
    std::optional<visited_edge> find_edge(InputIt first, InputIt last) const
    {
        if (nodes_.empty())
            return std::nullopt;
        visited_edge result{};  // root
        while (true) {
            auto str = substr(result.child);
            result.path_len += step::size(str);
            auto diff = std::mismatch(first, last, begin(str), end(str), eq_);
            if (diff.first == last)
                return result;
            if (diff.second != end(str) || leaf(result.child))
                return std::nullopt;
            auto& children = nodes_[result.child].children;
            auto it = children.find(*diff.first);
            if (it == children.end())
                return std::nullopt;
            first = diff.first;
            result.parent = std::exchange(result.child, it->second);
        }
    }

    void spread(std::stack<visited_edge>& dest, const visited_edge& src) const
    {
        std::stack<visited_edge> reverse;
        for (auto& [key, grandchild] : nodes_[src.child].children)
            reverse.push({src.child,
                          grandchild,
                          Size(src.path_len + step::size(substr(grandchild)))});
        for (; !reverse.empty(); reverse.pop())
            dest.push(reverse.top());
    }

    template <class Visitor>
    void dfs(const visited_edge& edge, Visitor visitor) const
    {
        for (std::stack<visited_edge> stack{{edge}}; !stack.empty();) {
            auto& top = stack.top();
            visitor(static_cast<const visited_edge&>(top));
            if (top.visited)
                stack.pop();
            else {
                top.visited = true;
                if (!leaf(top.child))
                    spread(stack, top);
            }
        }
    }
};

}  // namespace step

#endif  // STEP_SUFFIX_TREE_HPP
