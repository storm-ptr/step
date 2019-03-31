<img align="right" src="https://user-images.githubusercontent.com/3381451/40880432-5b9e7086-66b9-11e8-9718-4b1ea4eae317.png" width="25%">

[![Build Status](https://travis-ci.org/storm-ptr/step.svg?branch=master)](https://travis-ci.org/storm-ptr/step)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/storm-ptr/step?svg=true&branch=master)](https://ci.appveyor.com/project/storm-ptr/step/branch/master)

Step is a library of STL-like algorithms and data structures (C++17, header-only).

Algorithms:
* <details><summary>edit distance</summary><p>

  [wiki](https://en.wikipedia.org/wiki/Levenshtein_distance)
  ```C++
  pairs_t pairs;
  step::edit_distance::join("this"sv, "has"sv, std::back_inserter(pairs));
  CHECK(pairs ==
        pairs_t{{'t', std::nullopt}, {'h', 'h'}, {'i', 'a'}, {'s', 's'}});
  ```
  </p></details>
* <details><summary>longest common subsequence</summary><p>
  
  [wiki](https://en.wikipedia.org/wiki/Longest_common_subsequence_problem)
  ```C++
  std::string str;
  step::longest_common_subsequence::intersection("LCS is the basis of "sv,
                                                 "the diff utility"sv,
                                                 std::back_inserter(str));
  CHECK(str == "the if ");
  ```
  </p></details>
* <details><summary>longest common substring</summary><p>

  [wiki](https://en.wikipedia.org/wiki/Longest_common_substring_problem)
  ```C++
  auto range = step::longest_common_substring::find_with_suffix_tree(
      "the longest string that is #", "a substring of two strings $");
  CHECK(" string" == std::string(range.first, range.second));
  ```
  </p></details>
* <details><summary>longest increasing subsequence</summary><p>

  [wiki](https://en.wikipedia.org/wiki/Longest_increasing_subsequence)
  ```C++
  std::vector v{6, 3, 4, 8, 10, 5, 7, 1, 9, 2};
  int expect[] = {3, 4, 5, 7, 9};
  auto it = step::longest_increasing_subsequence::partition(v);
  CHECK(std::equal(v.begin(), it, std::begin(expect), std::end(expect)));
  ```
  </p></details>
* <details><summary>longest repeated substring</summary><p>

  [wiki](https://en.wikipedia.org/wiki/Longest_repeated_substring_problem)
  ```C++
  auto range = step::longest_repeated_substring::find_with_suffix_array(
      "the longest substring of a string that occurs at least twice");
  CHECK("string " == std::string(range.first, range.second));
  ```
    </p></details>
* <details><summary>maximum subarray</summary><p>

  [wiki](https://en.wikipedia.org/wiki/Maximum_subarray_problem)
  ```C++
  int arr[] = {-2, -3, 4, -1, -2, 1, 5, -3};
  std::array expect{4, -1, -2, 1, 5};
  auto range = step::maximum_subarray::find(arr);
  CHECK(std::equal(range.first, range.second, expect.begin(), expect.end()));
  ```
  </p></details>

Data structures:
* <details><summary>suffix array</summary><p>

  [wiki](https://en.wikipedia.org/wiki/Suffix_array)
  ```C++
  auto str = "how can I quickly search for text within a document?"sv;
  step::suffix_array arr{str};
  CHECK(arr.find("quick"sv) == 10);
  ```
  </p></details>
* <details><summary>suffix tree</summary><p>

  [wiki](https://en.wikipedia.org/wiki/Suffix_tree)
  ```C++
  auto str = "use the quick find feature to search for a text"sv;
  step::suffix_tree tree{};
  std::copy(str.begin(), str.end(), std::back_inserter(tree));
  CHECK(tree.find("quick"sv) == 8);
  ```
  </p></details>

to be continued
