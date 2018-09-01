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
 * @param T - type of the stored elements;
 * @param Map - type of associative container to store the edges,
 * its key_type shall be T (boost::container::flat_map e.g.).
 * @param Len - type is used to specify the maximum number of elements.
 * @see https://en.wikipedia.org/wiki/Suffix_tree
 */
template <class T, template <class...> class Map = std::map, class Len = size_t>
class suffix_tree {
public:
    using value_type = T;
    using size_type = Len;
    static constexpr auto npos = std::numeric_limits<size_type>::max();

    struct substring {
        size_type pos = 0;
        size_type len = 0;
    };

    auto data() const { return str_.data(); }
    size_type size() const { return str_.size(); }
    auto begin(const substring& str) const { return data() + str.pos; }
    auto end(const substring& str) const { return begin(str) + str.len; }

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
                if (str_[pos(edge) + reminder(pos_) - 1] == str_.back()) {
                    set_link_to(link_);
                    break;
                }
                set_link_to(split(edge));
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
     * Find the first occurrence of the pattern as substring in O(M) time,
     * where M is length of the pattern.
     */
    template <typename InputIt>
    auto find(InputIt first, InputIt last) const
    {
        auto[prefix_sz, edge] = find_edge(first, last);
        return edge == npos ? npos : pos(edge) - prefix_sz;
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
        dfs(find_edge(first, last), [&](auto prefix_sz, auto& str) {
            if (str.pos + str.len == size())  // suffix
                result.push_back(str.pos - prefix_sz);
        });
        return result;
    }

    /**
     * Depth-first traversal of the nodes.
     * Signature of @param vis is (size_type prefix_sz, const substring& str):
     * @param prefix_sz - length of all substrings
     * found on the path from the root to the node;
     * @param str - position and length of the node substring.
     */
    template <typename Visitor>
    void visit(const Visitor& vis) const
    {
        dfs({0, nodes_.empty() ? npos : 0}, vis);
    }

private:
    struct node {
        substring str;
        Map<T, size_type> edges;
        size_type link = 0;
    };

    using prefix_and_edge = std::pair<size_type, size_type>;

    std::vector<T> str_;
    std::vector<node> nodes_;  // inner only
    size_type pos_;
    size_type link_;

    static size_type inverse(size_type idx) { return npos - idx - 1; }
    size_type reminder(size_type pos) const { return size() - pos; }
    size_type pos(size_type edge) const { return substr(edge).pos; }

    substring substr(size_type edge) const
    {
        if (edge < nodes_.size())
            return nodes_[edge].str;
        auto pos = inverse(edge);
        return {pos, reminder(pos)};
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
        auto len = substr(edge).len;
        if (reminder(pos_) <= len)
            return false;
        pos_ += len;
        link_ = edge;
        return true;
    }

    size_type split(size_type& edge)
    {
        auto[pos, len] = substring{this->pos(edge), reminder(pos_) - 1};
        size_type link = nodes_.size();
        if (edge >= nodes_.size())
            edge = link;
        auto result = edge;
        nodes_.push_back({{pos, len}, {{str_.back(), inverse(size() - 1)}}});
        if (result == link)
            nodes_[result].edges.emplace(str_[pos + len], inverse(pos + len));
        else {
            nodes_[result].str.pos += len;
            nodes_[result].str.len -= len;
            std::swap(nodes_[result], nodes_.back());
            nodes_[result].edges.emplace(str_[pos + len], link);
        }
        return result;
    }

    template <typename InputIt>
    prefix_and_edge find_edge(InputIt first, InputIt last) const
    {
        if (nodes_.empty())
            return {0, npos};
        auto[prefix_sz, edge] = prefix_and_edge{0, 0};
        while (true) {
            auto str = substr(edge);
            auto diff = std::mismatch(first, last, begin(str), end(str));
            if (diff.first == last)
                return {prefix_sz, edge};
            if (diff.second != end(str) || edge >= nodes_.size())
                return {0, npos};
            auto it = nodes_[edge].edges.find(*diff.first);
            if (it == nodes_[edge].edges.end())
                return {0, npos};
            first = diff.first;
            prefix_sz += str.len;
            edge = it->second;
        }
    }

    template <typename Visitor>
    void dfs(const prefix_and_edge& start, const Visitor& vis) const
    {
        std::stack<prefix_and_edge> stack;
        if (start.second != npos)
            stack.push(start);
        while (!stack.empty()) {
            auto[prefix_sz, edge] = stack.top();
            stack.pop();
            auto str = substr(edge);
            vis(prefix_sz, str);
            if (edge < nodes_.size()) {
                decltype(stack) reversed;
                for (auto & [ key, edge ] : nodes_[edge].edges)
                    reversed.push({prefix_sz + str.len, edge});
                for (; !reversed.empty(); reversed.pop())
                    stack.push(reversed.top());
            }
        }
    }
};

}  // namespace step

#endif  // STEP_SUFFIX_TREE_HPP
