// Andrew Naplavkov

#ifndef STEP_SUFFIX_TREE_HPP
#define STEP_SUFFIX_TREE_HPP

#include <functional>
#include <iterator>
#include <limits>
#include <stack>
#include <unordered_map>
#include <vector>

namespace step {

/**
 * Ukkonen's online algorithm for constructing suffix tree.
 * Time complexity O(N*log(N)), space complexity O(N),
 * where N is length of stirng.
 * @param T - type of the characters;
 * @param Size - to specify the maximum number of characters;
 * @param Map - an associative container that is used to to store the edges,
 * its key_type shall be T (boost::container::flat_map e.g.);
 * @param Equal - to determine whether two characters are equivalent.
 * @see https://en.wikipedia.org/wiki/Suffix_tree
 */
template <class T = char,
          class Size = size_t,
          template <class...> class Map = std::unordered_map,
          class Equal = std::equal_to<>>
class suffix_tree {
public:
    using value_type = T;
    using substring = std::pair<Size, Size>;  // position range
    static constexpr Size npos = std::numeric_limits<Size>::max();

    auto data() const { return str_.data(); }
    Size size() const { return str_.size(); }

    static Size size(const substring& str) { return str.second - str.first; }
    auto begin(const substring& str) const { return data() + str.first; }
    auto end(const substring& str) const { return data() + str.second; }
    bool suffix(const substring& str) const { return str.second == size(); }

    void clear() noexcept
    {
        str_.clear();
        nodes_.clear();
        pos_ = 0;
        link_ = 0;
    }

    void reserve(Size len)
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
                    return set_link_to(link_);  // rule 3
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
    template <class InputIt>
    Size find(InputIt first, InputIt last) const
    {
        auto p = find_path(first, last);
        return substr(p.link).second - p.len;
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
    template <class InputIt>
    auto find_all(InputIt first, InputIt last) const
    {
        std::vector<Size> result;
        dfs(find_path(first, last),
            [&](const auto& str, const auto&, auto len) {
                if (suffix(str))
                    result.push_back(str.second - len);
            },
            [](auto&&...) {});
        return result;
    }

    template <class InputRng>
    auto find_all(const InputRng& rng) const
    {
        return find_all(std::begin(rng), std::end(rng));
    }

    /**
     * Depth-first traversal of the nodes.
     * @param pre and @param post visitors have signature:
     * @param str - substring of the node;
     * @param parent_str - substring of the parent node;
     * @param len - number of characters on the path from the root to the node.
     */
    template <class PreVisit, class PostVisit>
    void visit(PreVisit pre, PostVisit post) const
    {
        dfs(nodes_.empty() ? path{npos} : path{}, pre, post);
    }

private:
    struct node {
        Map<T, Size> edges;
        substring str;
        Size link;
    };

    struct path {
        Size link;
        Size parent_link;
        Size len;
        bool visited;
    };

    std::vector<T> str_;
    std::vector<node> nodes_;  // internal nodes
    Size pos_ = 0;
    Size link_ = 0;

    static Size inverse(Size n) { return npos - n - 1; }
    Size reminder(Size pos) const { return size() - pos; }
    bool leaf(Size link) const { return link >= nodes_.size(); }

    substring substr(Size link) const
    {
        return leaf(link) ? substring{inverse(link), size()} : nodes_[link].str;
    }

    auto make_linker()
    {
        return [prev = npos, this](Size link) mutable {
            if (prev != npos)
                nodes_[prev].link = link;
            prev = link;
        };
    }

    bool walk_down(Size link)
    {
        auto len = size(substr(link));
        if (reminder(pos_) <= len)
            return false;
        pos_ += len;
        link_ = link;
        return true;
    }

    Size split(Size& edge)
    {
        auto str = substr(edge);
        substring head{str.first, str.first + reminder(pos_) - 1};
        substring tail{head.second, str.second};
        if (Equal{}(str_[tail.first], str_.back()))
            return npos;
        if (leaf(edge))
            edge = nodes_.size();
        auto result = edge;
        nodes_.push_back({{{str_.back(), inverse(size() - 1)}}, head});
        if (result == nodes_.size() - 1)
            nodes_[result].edges[str_[tail.first]] = inverse(tail.first);
        else {
            nodes_[result].str = tail;
            std::swap(nodes_[result], nodes_.back());
            nodes_[result].edges[str_[tail.first]] = nodes_.size() - 1;
        }
        return result;
    }

    template <class InputIt>
    path find_path(InputIt first, InputIt last) const
    {
        if (nodes_.empty())
            return {npos};
        Equal equal{};
        path p{};  // root
        while (true) {
            auto str = substr(p.link);
            p.len += size(str);
            auto diff = std::mismatch(first, last, begin(str), end(str), equal);
            if (diff.first == last)
                return p;
            if (diff.second != end(str) || leaf(p.link))
                return {npos};
            auto it = nodes_[p.link].edges.find(*diff.first);
            if (it == nodes_[p.link].edges.end())
                return {npos};
            first = diff.first;
            p.parent_link = p.link;
            p.link = it->second;
        }
    }

    template <class PreVisit, class PostVisit>
    void dfs(const path& root, PreVisit pre, PostVisit post) const
    {
        std::stack<path> stack, tmp;
        if (root.link != npos)
            stack.push(root);
        while (!stack.empty()) {
            if (auto& p = stack.top(); p.visited) {
                post(substr(p.link), substr(p.parent_link), p.len);
                stack.pop();
            }
            else {
                pre(substr(p.link), substr(p.parent_link), p.len);
                p.visited = true;
                if (!leaf(p.link)) {
                    for (auto& [key, edge] : nodes_[p.link].edges)
                        tmp.push(
                            {edge, p.link, Size(p.len + size(substr(edge)))});
                    for (; !tmp.empty(); tmp.pop())
                        stack.push(tmp.top());
                }
            }
        }
    }
};

}  // namespace step

#endif  // STEP_SUFFIX_TREE_HPP
