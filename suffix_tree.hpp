// Andrew Naplavkov

#ifndef STEP_SUFFIX_TREE_HPP
#define STEP_SUFFIX_TREE_HPP

#include <limits>
#include <map>
#include <stack>
#include <stdexcept>
#include <vector>

namespace step {

/**
 * Ukkonen's online algorithm for constructing suffix tree.
 * Time complexity O(N*log(N)), space complexity O(N),
 * where N is length of stirng.
 * @param T - type of the stored characters;
 * @param Map - type of associative container to store the edges,
 * its key_type shall be T (boost::container::flat_map e.g.);
 * @param Len - type is used to specify the maximum number of characters.
 * @see https://en.wikipedia.org/wiki/Suffix_tree
 */
template <class T, template <class...> class Map = std::map, class Len = size_t>
class suffix_tree {
public:
    using value_type = T;
    using size_type = Len;
    static constexpr auto npos = std::numeric_limits<size_type>::max();
    auto data() const { return str_.data(); }
    size_type size() const { return str_.size(); }

    void clear() noexcept
    {
        str_.clear();
        nodes_.clear();
        pos_ = 0;
        link_ = 0;
    }

    void reserve(size_type len)
    {
        str_.reserve(len);
        nodes_.reserve(len);
    }

    /// Basic exception guarantee.
    void push_back(T val) try {
        str_.push_back(std::move(val));
        if (nodes_.empty())
            nodes_.emplace_back();  // root
        auto set_link_to = make_linker();
        while (reminder(pos_)) {
            if (auto& edge = nodes_[link_].edges[str_[pos_]]; edge) {
                if (walk_down(edge))
                    continue;
                if (auto link = split(edge); link != npos)
                    set_link_to(link);
                else
                    return set_link_to(link_);
            }
            else {
                edge = inverse(pos_);
                set_link_to(link_);
            }
            if (link_)
                link_ = nodes_[link_].link;
            else
                ++pos_;
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
    template <typename InputIt>
    size_type find(InputIt first, InputIt last) const
    {
        auto[prefix_len, edge] = find_edge(first, last);
        return edge == npos ? npos : substr(edge).first - prefix_len;
    }

    /**
     * Find all occurrences of the pattern for explicit suffix tree
     * (padded with a terminal symbol not seen in the string).
     */
    template <typename InputIt>
    auto find_all(InputIt first, InputIt last) const
    {
        if (reminder(pos_))
            throw std::logic_error{"implicit suffix_tree"};
        std::vector<size_type> result;
        dfs(find_edge(first, last), [&](auto first, auto nodal, auto last) {
            if (last == size())  // suffix
                result.push_back(first);
        });
        return result;
    }

    /**
     * Depth-first traversal of the nodes.
     * @param vis gets the concatenation of all substrings found on the
     * path from the root to the node given as:
     * @param first - start offset of the substring;
     * @param nodal - offset of the nodal part (end of the prefix);
     * @param last - past-the-last.
     */
    template <typename Visitor>
    void visit(const Visitor& vis) const
    {
        dfs({0, nodes_.empty() ? npos : 0}, vis);
    }

private:
    using substring = std::pair<size_type, size_type>;
    using prefix_len_and_edge = std::pair<size_type, size_type>;

    struct node {
        substring str;
        Map<T, size_type> edges;
        size_type link;
    };

    std::vector<T> str_;
    std::vector<node> nodes_;  // inner only
    size_type pos_;
    size_type link_;

    static auto inverse(size_type idx) { return npos - idx - 1; }
    static auto size(const substring& str) { return str.second - str.first; }
    auto begin(const substring& str) const { return data() + str.first; }
    auto end(const substring& str) const { return data() + str.second; }
    auto reminder(size_type pos) const { return size() - pos; }
    auto leaf(size_type edge) const { return edge >= nodes_.size(); }

    auto substr(size_type edge) const
    {
        return leaf(edge) ? substring{inverse(edge), size()} : nodes_[edge].str;
    }

    auto make_linker()
    {
        return [ prev = npos, this ](size_type link) mutable
        {
            if (prev != npos)
                nodes_[prev].link = link;
            prev = link;
        };
    }

    bool walk_down(size_type edge)
    {
        auto len = size(substr(edge));
        if (reminder(pos_) <= len)
            return false;
        pos_ += len;
        link_ = edge;
        return true;
    }

    size_type split(size_type& edge)
    {
        auto str = substr(edge);
        substring head{str.first, str.first + reminder(pos_) - 1};
        substring tail{head.second, str.second};
        if (str_[tail.first] == str_.back())
            return npos;
        if (leaf(edge))
            edge = nodes_.size();
        auto result = edge;
        nodes_.push_back({head, {{str_.back(), inverse(size() - 1)}}});
        if (result == nodes_.size() - 1)
            nodes_[result].edges[str_[tail.first]] = inverse(tail.first);
        else {
            nodes_[result].str = tail;
            std::swap(nodes_[result], nodes_.back());
            nodes_[result].edges[str_[tail.first]] = nodes_.size() - 1;
        }
        return result;
    }

    template <typename InputIt>
    prefix_len_and_edge find_edge(InputIt first, InputIt last) const
    {
        if (nodes_.empty())
            return {0, npos};
        auto[prefix_len, edge] = prefix_len_and_edge{0, 0};
        while (true) {
            auto str = substr(edge);
            auto diff = std::mismatch(first, last, begin(str), end(str));
            if (diff.first == last)
                return {prefix_len, edge};
            if (diff.second != end(str) || leaf(edge))
                return {0, npos};
            auto it = nodes_[edge].edges.find(*diff.first);
            if (it == nodes_[edge].edges.end())
                return {0, npos};
            first = diff.first;
            prefix_len += size(str);
            edge = it->second;
        }
    }

    template <typename Visitor>
    void dfs(const prefix_len_and_edge& origin, const Visitor& vis) const
    {
        std::stack<prefix_len_and_edge> stack;
        decltype(stack) reversed;
        if (origin.second != npos)
            stack.push(origin);
        while (!stack.empty()) {
            auto[prefix_len, edge] = stack.top();
            stack.pop();
            auto str = substr(edge);
            vis(str.first - prefix_len, str.first, str.second);
            if (!leaf(edge)) {
                for (auto & [ key, edge ] : nodes_[edge].edges)
                    reversed.push({prefix_len + size(str), edge});
                for (; !reversed.empty(); reversed.pop())
                    stack.push(reversed.top());
            }
        }
    }
};

}  // namespace step

#endif  // STEP_SUFFIX_TREE_HPP
