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
 * Time complexity O(N*log(N)), space complexity O(N),
 * where N is length of text.
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

    auto size() const { return (Size)str_.size(); }
    auto data() const { return str_.data(); }
    static Size size(const substring& str) { return str.second - str.first; }
    auto begin(const substring& str) const { return data() + str.first; }
    auto end(const substring& str) const { return data() + str.second; }
    bool suffix(const substring& str) const { return str.second == size(); }

    void clear() noexcept
    {
        str_.clear();
        nodes_.clear();
        active_char_ = 0;
        active_node_ = 0;
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
            if (auto& edge = nodes_[active_node_].edges[str_[active_char_]]) {
                if (walk_down(edge))
                    continue;
                if (!split(edge))
                    return connect(active_node_);
                connect(nodes() - 1);
            }
            else {
                edge = flip(active_char_);
                connect(active_node_);
            }
            if (active_node_)  // not root
                active_node_ = nodes_[active_node_].link;
            else
                ++active_char_;
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
        auto way = find_path(first, last);
        return way ? substr(way->node).second - way->len : size();
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
        if (auto way = find_path(first, last))
            dfs(*way,
                [&](const auto& node_str, const auto&, auto len) {
                    if (suffix(node_str))
                        *result++ = node_str.second - len;
                },
                [](auto&&...) {});
        return result;
    }

    template <class InputRng, class OutputIt>
    OutputIt find_all(const InputRng& rng, OutputIt result) const
    {
        return find_all(std::begin(rng), std::end(rng), result);
    }

    /**
     * Depth-first traversal of the nodes.
     * @param pre and @param post visitors have signature:
     * @param node_str - substring of the node;
     * @param parent_str - substring of the parent node;
     * @param len - number of characters on the path from the root to the node.
     */
    template <class PreVisitor, class PostVisitor>
    void visit(PreVisitor&& pre, PostVisitor&& post) const
    {
        if (nodes_.empty())
            return;
        dfs({}, std::forward<PreVisitor>(pre), std::forward<PostVisitor>(post));
    }

private:
    inline static const auto eq_ = key_equal_or_equivalence_t<Map<T, Size>>{};

    struct internal_node {
        Map<T, Size> edges;
        substring str;
        Size link;
    };

    std::vector<T> str_;
    std::vector<internal_node> nodes_;
    Size active_char_ = 0;
    Size active_node_ = 0;

    static Size flip(Size n) { return std::numeric_limits<Size>::max() - n; }
    Size reminder() const { return size() - active_char_; }
    auto nodes() const { return (Size)nodes_.size(); }
    bool leaf(Size node) const { return node >= nodes(); }

    auto substr(Size node) const
    {
        return leaf(node) ? substring{flip(node), size()} : nodes_[node].str;
    }

    auto connector()
    {
        if (nodes_.empty())
            nodes_.emplace_back();  // root
        return [this, last = nodes()](Size node) mutable {
            if (last < nodes() && last != node)
                nodes_[last++].link = node;
        };
    }

    bool walk_down(Size node)
    {
        auto len = size(substr(node));
        if (reminder() <= len)
            return false;
        active_char_ += len;
        active_node_ = node;
        return true;
    }

    bool split(Size& edge)
    {
        auto str = substr(edge);
        auto head = substring{str.first, str.first + reminder() - 1};
        auto tail = substring{head.second, str.second};
        if (eq_(str_[tail.first], str_.back()))
            return false;
        Size node = edge;
        edge = nodes();
        nodes_.push_back({{{str_.back(), flip(size() - 1)}}, head});
        if (leaf(node))
            nodes_.back().edges[str_[tail.first]] = flip(tail.first);
        else {
            nodes_.back().edges[str_[tail.first]] = node;
            nodes_[node].str = tail;
        }
        return true;
    }

    struct path {
        Size node;
        Size parent;
        Size len;
        bool visited;
    };

    template <class InputIt>
    std::optional<path> find_path(InputIt first, InputIt last) const
    {
        if (nodes_.empty())
            return std::nullopt;
        path result{};  // root
        while (true) {
            auto str = substr(result.node);
            result.len += size(str);
            auto diff = std::mismatch(first, last, begin(str), end(str), eq_);
            if (diff.first == last)
                return result;
            if (diff.second != end(str) || leaf(result.node))
                return std::nullopt;
            auto& edges = nodes_[result.node].edges;
            auto it = edges.find(*diff.first);
            if (it == edges.end())
                return std::nullopt;
            first = diff.first;
            result.parent = result.node;
            result.node = it->second;
        }
    }

    void push_edges(std::stack<path>& dest, const path& src) const
    {
        std::stack<path> reverse;
        for (auto& [key, edge] : nodes_[src.node].edges)
            reverse.push({edge, src.node, Size(src.len + size(substr(edge)))});
        for (; !reverse.empty(); reverse.pop())
            dest.push(reverse.top());
    }

    template <class PreVisitor, class PostVisitor>
    void dfs(const path& way, PreVisitor pre, PostVisitor post) const
    {
        for (std::stack<path> stack{{way}}; !stack.empty();)
            if (auto& top = stack.top(); top.visited) {
                post(substr(top.node), substr(top.parent), top.len);
                stack.pop();
            }
            else {
                pre(substr(top.node), substr(top.parent), top.len);
                top.visited = true;
                if (!leaf(top.node))
                    push_edges(stack, top);
            }
    }
};

}  // namespace step

#endif  // STEP_SUFFIX_TREE_HPP
