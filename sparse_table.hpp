// Andrew Naplavkov

#ifndef STEP_SPARSE_TABLE_HPP
#define STEP_SPARSE_TABLE_HPP

#include <cmath>
#include <step/common.hpp>
#include <vector>

namespace step {

/**
 * A data structure, that allows answering static range queries in O(1) time
 * with O(N*log(N)) preprocessing time and space, where:
 * N = std::distance(first, last).
 *
 * @param BinaryOp (minimum for example) is
 * - associative: op(a, op(b,c)) == op(op(a,b), c);
 * - commutative: op(a,b) == op(b,a);
 * - idempotent: op(a,a) == a.
 *
 * @see https://www.geeksforgeeks.org/sparse-table/
 */
template <typename T, typename BinaryOp = min>
class sparse_table {
private:
    BinaryOp op_;
    std::vector<std::vector<T>> data_;

public:
    template <typename InputIt>
    sparse_table(InputIt first, InputIt last, const BinaryOp& op = BinaryOp{})
        : op_{op}
    {
        data_.emplace_back(first, last);
        for (size_t exp = 1; exp < data_.back().size(); exp *= 2) {
            size_t h = data_.size();
            size_t size = data_.back().size() - exp;
            data_.emplace_back().reserve(size);
            for (size_t pos = 0; pos < size; ++pos)
                data_[h].push_back(
                    op_(data_[h - 1][pos], data_[h - 1][pos + exp]));
        }
    }

    size_t size() const { return data_.front().size(); }

    T subarray(size_t pos, size_t count) const
    {
        size_t h = std::log2(count);
        return op_(data_[h][pos], data_[h][pos + count - std::exp2(h)]);
    }
};

}  // namespace step

#endif  // STEP_SPARSE_TABLE_HPP
