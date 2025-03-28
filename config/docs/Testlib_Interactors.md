Interactive problems are problems in which solution talks to the judge. For example, [100553G - Gomoku](https://codeforces.com/gym/100553/problem/G "2014-2015 ACM-ICPC Northeastern European Regional Contest (NEERC 14)"). We don't see interactive problems much in ACM-ICPC style problems. Most of them are Olympiad style(IOI and CEOI). Unfortunately using interactive in Codeforces contests is not allowed, but you can see some of them in Gym. Also [Polygon](https://polygon.codeforces.com/) handles such problems(there's a checkbox `Interactive` in general info of the problem). When we don't wanna handle the judge manually, we should use a code named interactor to talk to code instead of a person. With testlib.h, we can write interactors as simple as checkers and validators.

In an interactive problem, you may use also a checker. To connect this programs together(generator, validator, solution, checker and interactor), you can use teslib input streams. An input stream, is a structure that reads data from a specific file using some pre-implemented methods. Input streams you can use with testlib.h:

1.  `inf`: It's the input generated by generator or manually (In polygon, manual tests and output of generators, based on how the input file of the current testcase was generated).
2.  `ouf`: It's the output produced by the solution you're working on.
3.  `ans`: Output produced by your correct solution.

Also, there's an input/output stream for interactive tasks named `tout`. It's a log file, you can write some information to it with the interactor and later, check the information written in it with the checker (and determine the verdict). For writing in it, you can use style of C++ `cout`, like `tout << n << endl;`. In the checker, you can read that information from `ouf`.

Methods you can use for input streams: [Validator doc](https://codeforces.com/blog/entry/18426)

In interactor, you read the information about the current testcase from `inf`, write what needs to be given to the solution you're checking and the correct solution using stdout (online), read the output produces by the solution you're checking using `ouf` (online), read the output produces by your correct solution using `ans` (online) and write log to `tout` if you want.

If at anytime, some with methods of input streams used in interactor goes wrong(fails), verdict will be Wrong Answer.

Also, you can determine the verdict in interactor. There are much useful methods in teslib you can use in interactors for `assert`\-like checking, ensuring and determining the verdict. You can find them in [checker docs](https://codeforces.com/blog/entry/18431) (methods like `quitf` and `ensuref`).

You can also see possible verdicts in [checker docs](https://codeforces.com/blog/entry/18431).

If verdict determined by interactor's ok, then it will be ensured by the checker (which uses `tout`/`ouf`) if there's any.

**How to use interactor program ?**

Simple:

### Sample Interactive Problem

I(judge) choose an integer in the interval \[1, 10<sup>9</sup>\] and you should write a code to guess it. You can ask me at most 50 questions. In each question, you tell me a number in the interval \[1, 10<sup>9</sup>\], and I tell you:

-   1 if it is equal to answer(the chosen number), and your program should stop asking after that.
-   0 if it is smaller than answer.
-   2 if it is greater than answer.

**Sample interactor for this problem:**

**Note:** Like checkers and validators and generator, you should first initialize your interactor with `registerInteraction(argc, argv)`.

Please note that in this problem, we can determine the verdict without using the correct solution and `ans` because we don't care about it's product. But in some problems, we'll have to compare it with the product of the correct solution using `ans`.

Resources: [Checkers](https://codeforces.com/blog/entry/18426), [validators](https://codeforces.com/blog/entry/18426) and my personal experience from reading one of [MikeMirzayanov](https://codeforces.com/profile/MikeMirzayanov "Headquarters, MikeMirzayanov")'s interactors.