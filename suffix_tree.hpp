// Andrew Naplavkov

#ifndef STEP_SUFFIX_TREE_HPP
#define STEP_SUFFIX_TREE_HPP

#include <map>
#include <stack>
#include <stdexcept>
#include <vector>

namespace step {

/**
 * Ukkonen's online algorithm for constructing suffix tree.
 * Time complexity O(N*log(N)), space complexity O(N),
 * where N is length of stirng.
 *
 * Template parameters:
 * T - type of the stored elements;
 * Map - type of associative container to store the edges,
 *   its key_type shall be T (boost::container::flat_map e.g.).
 *
 * @see https://en.wikipedia.org/wiki/Suffix_tree
 */
template <class T = char, template <class...> class Map = std::map>
class suffix_tree {
public:
    using value_type = T;
    static constexpr size_t npos = -1;
    auto data() const { return str_.data(); }
    auto size() const { return str_.size(); }

    void clear() noexcept
    {
        str_.clear();
        nodes_.clear();
        active_ = 0;
        remainder_ = {0, npos};
    }

    void reserve(size_t sz)
    {
        str_.reserve(sz);
        nodes_.reserve(2 * sz);
    }

    /**
     * Basic exception guarantee.
     */
    void push_back(T val) try {
        str_.push_back(std::move(val));
        if (nodes_.empty())
            nodes_.emplace_back(substr{0, 0});  // root
        auto set_link_to = make_linker();
        while (size(remainder_)) {
            if (auto n = nodes_[active_].edges[*begin(remainder_)]; n) {
                if (walk_down(n))
                    continue;
                if (*(begin(nodes_[n]) + size(remainder_) - 1) == str_.back()) {
                    set_link_to(active_);
                    break;
                }
                split(n);
                set_link_to(n);
            }
            else {
                nodes_.emplace_back(remainder_);
                add_edge_from(active_);
                set_link_to(active_);
            }
            if (active_)
                active_ = nodes_[active_].link;
            else
                remainder_.remove_prefix(1);
        }
    }
    catch (...) {
        clear();
        throw;
    }

    /**
     * Find the first occurrence of the pattern as substring in O(M) time,
     * where M = is length of the pattern.
     */
    template <typename InputIt>
    size_t find(InputIt first, InputIt last) const
    {
        auto[prefix_sz, n] = find_node(first, last);
        return n == npos ? npos : nodes_[n].pos - prefix_sz;
    }

    /**
     * Find all occurrences of the pattern for explicit suffix tree
     * (padded with a terminal symbol not seen in the string).
     */
    template <typename InputIt>
    auto find_all(InputIt first, InputIt last) const
    {
        if (size(remainder_))
            throw std::logic_error{"implicit suffix_tree"};
        std::vector<size_t> result;
        dfs(find_node(first, last),
            [&](size_t prefix_sz, size_t pos, size_t sz) {
                if (pos + sz == size())  // suffix
                    result.push_back(pos - prefix_sz);
            });
        return result;
    }

    /**
     * Depth-first traversal of the nodes.
     * Signature of the function object @param op:
     * (size_t prefix_sz, size_t pos, size_t sz),
     * where:
     * @param prefix_sz - length of all substrings
     *   found on the path from the root to the node;
     * @param pos, @param sz - position and length of the node substring.
     */
    template <typename Operation>
    void visit(const Operation& op) const
    {
        dfs(prefix_and_node{0, nodes_.empty() ? npos : 0}, op);
    }

private:
    struct substr {
        size_t pos;
        size_t count;

        void remove_prefix(size_t sz)
        {
            pos += sz;
            if (count != npos)
                count -= sz;
        }
    };

    struct node : substr {
        Map<T, size_t> edges;
        size_t link = 0;

        node(const substr& s) : substr{s} {}
    };

    using prefix_and_node = std::pair<size_t, size_t>;

    std::vector<T> str_;
    std::vector<node> nodes_;
    size_t active_ = 0;
    substr remainder_ = {0, npos};

    auto size(const substr& s) const
    {
        return s.count == npos ? size() - s.pos : s.count;
    }

    auto begin(const substr& s) const { return data() + s.pos; }
    auto end(const substr& s) const { return begin(s) + size(s); }

    auto make_linker()
    {
        return [ prev = npos, this ](size_t n) mutable
        {
            if (prev != npos)
                nodes_[prev].link = n;
            prev = n;
        };
    }

    void add_edge_from(size_t n)
    {
        nodes_[n].edges[*begin(nodes_.back())] = nodes_.size() - 1;
    }

    bool walk_down(size_t n)
    {
        auto sz = size(nodes_[n]);
        if (size(remainder_) <= sz)
            return false;
        active_ = n;
        remainder_.remove_prefix(sz);
        return true;
    }

    void split(size_t n)
    {
        nodes_.emplace_back(substr{nodes_[n].pos, size(remainder_) - 1});
        nodes_[n].remove_prefix(size(nodes_.back()));
        std::swap(nodes_[n], nodes_.back());
        add_edge_from(n);
        nodes_.emplace_back(substr{size() - 1, npos});
        add_edge_from(n);
    }

    template <typename InputIt>
    prefix_and_node find_node(InputIt first, InputIt last) const
    {
        if (nodes_.empty())
            return {0, npos};
        auto[prefix_sz, n] = prefix_and_node{0, 0};
        while (true) {
            auto diff =
                std::mismatch(first, last, begin(nodes_[n]), end(nodes_[n]));
            if (diff.first == last)
                return {prefix_sz, n};
            if (diff.second != end(nodes_[n]))
                return {0, npos};
            auto it = nodes_[n].edges.find(*diff.first);
            if (it == nodes_[n].edges.end())
                return {0, npos};
            first = diff.first;
            prefix_sz += size(nodes_[n]);
            n = it->second;
        }
    }

    template <typename Operation>
    void dfs(const prefix_and_node& start, const Operation& op) const
    {
        std::stack<prefix_and_node> stack;
        if (start.second != npos)
            stack.push(start);
        while (!stack.empty()) {
            auto[prefix_sz, n] = stack.top();
            stack.pop();
            op(prefix_sz, nodes_[n].pos, size(nodes_[n]));
            decltype(stack) reversed;
            for (auto & [ key, edge ] : nodes_[n].edges)
                reversed.push({prefix_sz + size(nodes_[n]), edge});
            for (; !reversed.empty(); reversed.pop())
                stack.push(reversed.top());
        }
    }
};

}  // namespace step

#endif  // STEP_SUFFIX_TREE_HPP
