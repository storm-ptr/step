// Andrew Naplavkov

#ifndef STEP_TEST_BLOB_HPP
#define STEP_TEST_BLOB_HPP

#include <step/blob.hpp>

TEST_CASE("blob")
{
    step::blob::container buf(42);
    step::blob::variant vars[] = {static_cast<int64_t>(1),
                                  3.14,
                                  "Hello, World!",
                                  static_cast<step::blob::view>(buf)};
    step::blob::container result;
    for (auto& var : vars)
        result << var;
    auto src = static_cast<step::blob::view>(result);
    for (auto& var : vars)
        CHECK(var == step::blob::read<step::blob::variant>(src));
    std::ostringstream os;
    os << '\n' << step::blob::table{{"a", "b"}, result};
    CHECK(os.str() == R"(
| a             | b        |
|---------------|----------|
|             1 |     3.14 |
| Hello, World! | 42 bytes |
)");
}

#endif  // STEP_TEST_BLOB_HPP
