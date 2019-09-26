// Andrew Naplavkov

#ifndef STEP_TEST_LONGEST_COMMON_SUBSEQUENCE_HPP
#define STEP_TEST_LONGEST_COMMON_SUBSEQUENCE_HPP

#include <sstream>
#include <step/example/diff/utility.hpp>
#include <step/longest_common_subsequence.hpp>
#include <step/test/case_insensitive.hpp>
#include <string_view>

TEST_CASE("longest_common_subsequence_hello_world")
{
    using namespace std::literals;
    std::string str;
    step::longest_common_subsequence::intersection("LCS is the basis of "sv,
                                                   "the diff utility"sv,
                                                   std::back_inserter(str));
    CHECK(str == "the if ");
}

TEST_CASE("longest_common_subsequence_intersection")
{
    struct {
        std::string_view lhs;
        std::string_view rhs;
        std::string_view expect;
    } tests[] = {{"XMJYAUZ", "MZJAWXU", "MJAU"},
                 {"AGGTAB", "GXTXAYB", "GTAB"},
                 {"ABCDGH", "aedfhr", "ADH"},
                 {"BANANA", "ATANA", "AANA"},
                 {"gac", "AGCAT", "ga"}};
    for (auto& [lhs, rhs, expect] : tests) {
        std::string str;
        step::longest_common_subsequence::intersection(
            lhs,
            rhs,
            std::back_inserter(str),
            step::case_insensitive::equal_to{});
        CHECK(str == expect);
    }
}

TEST_CASE("diff")
{
    const std::string str1 = R"(This part of the
document has stayed the
same from version to
version.  It shouldn't
be shown if it doesn't
change.  Otherwise, that
would not be helping to
compress the size of the
changes.

This paragraph contains
text that is outdated.
It will be deleted in the
near future.

It is important to spell
check this dokument. On
the other hand, a
misspelled word isn't
the end of the world.
Nothing in the rest of
this paragraph needs to
be changed. Things can
be added after it.)";

    const std::string str2 = R"(This is an important
notice! It should
therefore be located at
the beginning of this
document!

This part of the
document has stayed the
same from version to
version.  It shouldn't
be shown if it doesn't
change.  Otherwise, that
would not be helping to
compress the size of the
changes.

It is important to spell
check this document. On
the other hand, a
misspelled word isn't
the end of the world.
Nothing in the rest of
this paragraph needs to
be changed. Things can
be added after it.

This paragraph contains
important new additions
to this document.)";

    const std::string expect = R"(@@ -1,0 +1,6 @@
+This is an important
+notice! It should
+therefore be located at
+the beginning of this
+document!
+
@@ -11,5 +17,0 @@
-This paragraph contains
-text that is outdated.
-It will be deleted in the
-near future.
-
@@ -17,1 +18,1 @@
-check this dokument. On
+check this document. On
@@ -25,0 +26,4 @@
+
+This paragraph contains
+important new additions
+to this document.
)";

    auto lines1 = split(str1);
    auto lines2 = split(str2);
    std::ostringstream os;
    diff({lines1, 0, lines1.size()}, {lines2, 0, lines2.size()}, os);
    CHECK(os.str() == expect);
}

TEST_CASE("patience_diff")
{
    const std::string str1 = R"(.foo1 {
    margin: 0;
}

.bar {
    margin: 0;
})";

    const std::string str2 = R"(.bar {
    margin: 0;
}

.foo1 {
    margin: 0;
    color: green;
})";

    const std::string expect = R"(@@ -1,0 +1,4 @@
+.bar {
+    margin: 0;
+}
+
@@ -3,0 +7,1 @@
+    color: green;
@@ -4,4 +9,0 @@
-
-.bar {
-    margin: 0;
-}
)";

    auto lines1 = split(str1);
    auto lines2 = split(str2);
    std::ostringstream os;
    patience_diff({lines1, 0, lines1.size()}, {lines2, 0, lines2.size()}, os);
    CHECK(os.str() == expect);
}

#endif  // STEP_TEST_LONGEST_COMMON_SUBSEQUENCE_HPP
