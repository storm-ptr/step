// Andrew Naplavkov

#ifndef STEP_TEST_SPARSE_TABLE_HPP
#define STEP_TEST_SPARSE_TABLE_HPP

#include <step/sparse_table.hpp>

template <typename Range, typename SparseTable>
void check_sparse_table(const Range& rng, const SparseTable& tbl)
{
    REQUIRE(std::size(rng) == tbl.size());
    for (size_t i = 0; i < tbl.size(); ++i)
        CHECK(rng[i] == tbl.subarray(i, 1));
}

TEST_CASE("sparse_table_min")
{
    const int a[] = {7, 2, 3, 0, 5, 10, 3, 12, 18};
    step::sparse_table<int, step::min> tbl{std::begin(a), std::end(a)};
    check_sparse_table(a, tbl);
    CHECK(tbl.subarray(0, 4) == 0);
    CHECK(tbl.subarray(4, 3) == 3);
    CHECK(tbl.subarray(7, 1) == 12);
}

TEST_CASE("sparse_table_max")
{
    const int a[] = {2, 3, 5, 4, 6, 8};
    step::sparse_table<int, step::max> tbl{std::begin(a), std::end(a)};
    check_sparse_table(a, tbl);
    CHECK(tbl.subarray(0, 3) == 5);
    CHECK(tbl.subarray(3, 3) == 8);
    CHECK(tbl.subarray(1, 1) == 3);
}

TEST_CASE("sparse_table_gcd")
{
    const int a[] = {7, 2, 3, 0, 5, 10, 3, 12, 18};
    step::sparse_table<int, step::gcd> tbl{std::begin(a), std::end(a)};
    check_sparse_table(a, tbl);
    CHECK(tbl.subarray(0, 2) == 1);
    CHECK(tbl.subarray(1, 2) == 1);
    CHECK(tbl.subarray(4, 1) == 5);
}

#endif  // STEP_TEST_SPARSE_TABLE_HPP
