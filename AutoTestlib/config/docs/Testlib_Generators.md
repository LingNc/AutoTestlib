## What are generators?

Generators are helper programs that output test. Most programming task usually has a large input (for example, an array of up to 2 \* 10<sup>5</sup> elements, a tree of up to 10<sup>5</sup> vertices), so it's not possible to add all the tests manually. In these cases, generators come to the rescue.

If you are writing a generator in C++, it is recommended to use the [testlib.h](https://g...content-available-to-author-only...b.com/MikeMirzayanov/testlib) library.

## Types of generators

There are two types of generators:

1.  **Single-test generator:** output exactly one test in a single run. Usually, to generate several tests, one must run the generator several times with different command line parameters. Such generators output the test to the standard output stream (to the screen).
2.  **Multiple-test generator:** output many tests in a single run. Such generators output tests to files (one file for each test).

## An example single-test generator with testlib.h

The following generator output a pair of integers with each element from 1 to _n_ — where _n_ is a command line parameter passed to the generator.

On the surface, it seems that testlib.h is not necessary to write a generator. This is not true. Almost all generators need to generate random values, and it is tempted to use `rand()`. However, this is a bad practice. A basic principle of writing generators is that: a generator must output the same test when compiled by any compiler on any platform if it is run in the same way (using the same command line parameters). When using `rand()` or C++11 classes like `mt19937/uniform_int_distribution`, your program will output different tests when compiled with different compilers.

The random value generator in testlib.h ensures that the same value is generated regardless of the (test) generator and platform. Besides, testlib.h has various conveniences for generating tests, for example, `rnd.next("[a-z]{1,10}")` will return a random word of length 1 to 10 from letters a to z.

_Translator's note: There are more issues with using `rand()` aside from the above one. Refer to [this](https://c...content-available-to-author-only...s.com/profile/neal) blog for a detailed explanation about these issues._

## Available method

To initialize a testlib generator, the first line of your generator must be of the form `registerGen(argc, argv, 1);` (where 1 is the version of the random number generator used). After that, it will be possible to use the `rnd` object, which will be initialized with a hash from all the command line arguments. Thus, the output of `g 100` and `g.exe "100"` will be the same, while `g 100 0` will be different.

`rnd` is of type `random_t`. That is, you can create your own generator, but this is not necessary in most of the cases.

`rnd` has many useful member functions. Here are some examples:

| Call | Return value |
| --- | --- |
| rnd.next(4) | An equiprobable random integer from 0 to 3 (inclusive) |
| rnd.next(4, 100) | An equiprobable random integer from 4 to 100 (inclusive) |
| rnd.next(10.0) | An equiprobable random real number in the half-interval \[0;10) |
| rnd.next("one|two|three") | An equiprobable random word out of 'one', 'two' and 'three' |
| rnd.next("\[1-9\]\[0-9\]{99}") | An equiprobable random 100-digit number as a string |
| rnd.wnext(4,t) | `wnext` is a method of obtaining an uneven distribution (with a biased expectation), the parameter `t` denotes the number of calls to the maximum operation for similar next calls. For example `rnd.wnext(3, 1)` is equivalent to `max(rnd.next(3), rnd.next(3))`, and `rnd.wnext(4, 2)` is equivalent to `max(rnd.next(4), max(rnd.next(4), rnd.next(4)))`. If t < 0, then -t will find the minimum. If t = 0, then `wnext` is equivalent to `next`. |
| rnd.any(container) | A random element of the container `container` (with random access via an iterator), for example, it works for `std::vector` and `std::string` |

Also, please do not use `std::random_shuffle`, use the `shuffle` from testlib.h instead. It also takes two iterators, but works using `rnd`.

_Translator's note: If my understanding is correct, `rnd.wnext` is defined as follows:_

![](https://espresso.codeforces.com/55c44b13eed7b3451e3edea2986d5a541c62a279.png)

## Example: generating an undirected tree

Below is the code of an undirected tree generator that takes two parameters — the number of vertices and the 'elongation' of the tree. For example:

-   For _n_ = 10, _t_ = 1000, a path graph (degree of all vertices are at most 2) is likely to be generated
-   For _n_ = 10, _t_ =  - 1000, a star graph (there's only one non-leaf vertex in the tree) is likely to be generated.

## How to write a multiple-test generator?

A multiple-test generator in one execution can output more than one test. Tests by such a generator are output to files. In the generator on testlib.h it is enough to write `startTest(test_index)` before the test output. This will re-open (`freopen`) the standard output stream to a file named `test_index`.

Please note that if you are working with the Polygon system, in this case, you need to write something like `multigen a b c > {4-10}` in the script (if it is assumed that starting the multiple-test generator will return tests 4, 5, 6, 7, 8, 9, and 10).

## Other notes about generators

-   Strictly follow the format of the test — spaces and line breaks should be placed correctly. The test should end with a line feed. For example, if the test consists of a single number, then output it as `cout << rnd.next (1, n) << endl;` — with a line feed at the end.
-   If the test size is large, it is prefered to use `printf` instead of `cout` — this will improve the performance of the generator.
-   It is better to use `cout` to output `long long`, but if you want `printf`, then use the `I64` constant (for example, `printf(I64, x);`).
-   Please be aware about various cases of C++ [undefined behavior](https://e...content-available-to-author-only...a.org/wiki/Undefined_behavior). For example, in the first example generator above, if the two `cout` commands are combined into one, the order of the `rnd.next` function calls is not defined.

_Translator's note: about the third point, using `lld` constant with `printf` to output `long long` used to be problematic in the past, but is no longer an issue now._

## Further examples

Further examples of generators can be found in the [release notes](https://github.com/MikeMirzayanov/testlib/releases) or directly at [the GitHub repository](https://github.com/MikeMirzayanov/testlib/tree/master/generators).