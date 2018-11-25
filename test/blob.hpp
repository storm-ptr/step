// Andrew Naplavkov

#ifndef STEP_TEST_BLOB_HPP
#define STEP_TEST_BLOB_HPP

#include <step/blob.hpp>

TEST_CASE("blob")
{
    int64_t n = 1;
    double fp = 3.14;
    std::string_view sv = "Hello, World!";
    step::blob buf;
    step::write_variant(n, buf);
    auto bv = static_cast<step::blob_view>(buf);

    step::blob result;
    step::write_variant(n, result);
    step::write_variant(fp, result);
    step::write_variant(sv, result);
    step::write_variant(bv, result);

    auto vars = step::make_variants(static_cast<step::blob_view>(result));
    CHECK(std::get<int64_t>(vars[0]) == n);
    CHECK(std::get<double>(vars[1]) == fp);
    CHECK(std::get<std::string_view>(vars[2]) == sv);
    CHECK(std::get<step::blob_view>(vars[3]) == bv);

    std::ostringstream os;
    os << step::table{{"a", "b"}, vars};
    CHECK(os.str() == R"(| a             | b       |
|---------------|---------|
|             1 |    3.14 |
| Hello, World! | 9 bytes |
)");
}

#endif  // STEP_TEST_BLOB_HPP
