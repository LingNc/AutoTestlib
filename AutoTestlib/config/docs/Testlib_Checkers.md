## Introduction

Checker is a program that should be written when task allows more than one correct solution. Although it seems that it is very easy to write checker, there are lots of important technical details that are easy to forget if you don't use a special library like testlib.h.

A common convention is that a checker should be a program taking three command-line arguments: the testdata filename, the participant output filename and the jury answer filename. Checker should read the contents of the input, output and answer, decide whether participant answer is correct (and optimal being compared to the jury's answer if there can be unoptimal answer in this task) and return one of several pre-defined verdicts:

| Verdict | testlib macro | meaning |
| --- | --- | --- |
| Ok | `_ok` | The output is correct, follows the output format and represents an optimal answer (if applicable in this problem). |
| Wrong Answer | `_wa` | The output is wrong or incorrect. |
| Presentation Error | `_pe` | The output doesn't follow output format specification of the task. Although, on Codeforces this verdict is being replaced by Wrong Answer since it's usually hard to distinguish Presentation Error and Wrong Answer |
| Partially Correct | `_pc(score)` | If there is a partial scoring in the task, this verdict should be used in situation when solution is partially correct or to give solution some number of points. Here `score` should be an integer between 0 and 200 where 0 represents the lowest mark (no points) and 200 represents the highest mark (maximum possible number of points) |
| Fail | `_fail` | This verdict means that checker has encountered a critical internal error or that the jury's answer is incorrect or that contestant found the more optimal answer than jury. This verdict means that something wrong has happened and it requires a special investigation from jury. |

Usually the verdict returned by checker is indicated by the return code of its executable, but it may possibly be transfered to the testing system by many other ways: by creating a special xml-file with checker outcome, by writing to stdout or somehow else. When using testlib.h for writing a checker all those system-dependent ways are combined in a single expression `quitf(VERDICT, "comment", ...)`.

## Simplest checker example

**Problem statement**: You are given two integers _a_ and _b_ ( - 1000 ≤ _a_, _b_ ≤ 1000). Find their sum and output it.

Let's write a checker for this problem. It will be very simple:

## Available methods

There are lots of methods useful for writing checkers.

| Method | Description |
| --- | --- |
| `stream.readXXX` | All methods of form readXXX (like readInt, readLong, readDouble, readToken etc) are common for all testlib uses: checkers, validators and interactors. TODO: put all such methods on the separate page. |
| `void quit(TResult verdict, string message);`  
`void quit(TResult verdict, const char* message);`  
`void quitf(TResult verdict, const char* message, ...);` | Finishes the checker with a given verdict and comment. |
| `void quitif(bool condition, TResult verdict, const char* message, ...);` | if condition is true then performs quitf(verdict, message, ...) |
| `void ensuref(bool condition, const char* message, ...);` | An equivalent of assert. Checks that condition is true, otherwise finishes with \_fail verdict. Useful for debugging checkers. |

TODO: finish this list.

## readAns paradigm

Suppose you have a task that asks contestant to find a complex composite answer that is not just a single number. Example:

**Problem statement** You are given a connected undirected weighted graph witn _n_ vertices and _m_ edges. Find a simple path between vertex _s_ and vertex _t_ (_s_ ≠ _t_) of maximum weight and output it. Samples (input and output format is clarified in square brackets):

| Sample input | Sample output |
| --- | --- |
| 
4 5 \[n, m\]  
1 2 4 \[edges\]  
2 4 2  
1 4 4  
1 3 5  
3 4 3  
1 4 \[s, t\]

 | 

3 \[number of vertices in path\]  
1 3 4 \[path\]

 |

Here is an example of **bad** checker implementation for this task.

### Bad checker implementation

Here are the main two issues that appear in this checker.

1.  It believes that the jury's answer is absolutely correct. In case when jury's answer is unoptimal and contestant have really found the better answer, he will get the verdict WA that is not fair. There is a special verdict Fail exactly for this situation.
2.  It contains the duplicating code for extracting the answer value for jury and contestant. In this case extracting the value is just one "for" cycle but for a harder task it may be a very complicated subroutine, and as usual, using the duplicated code makes twice harder to fix it, rewrite it or change it when output format changes.

In fact, reading an answer value is a subroutine that works exactly the same for contestant and jury. That's why it is usually being put into a separate function receiving the input stream as a parameter.

### Good checker implementation

Notice that by using this paradigm we also check the jury answer for being correct. Checkers written in such form are usually shorter and easier to understand and fix. It is also applicable when task output is NO/(YES+certificate).

## Notes, advices and common mistakes

-   Use readAns paradigm. It really makes easier to understand and work with your checker.
-   Always use optional arguments for methods like readInt(), readLong() etc. If you forget to check range for some variable, your checker may work incorrectly or even face runtime-error that will result in Check Failed in testing system.

### Bad:

### Good:

-   Use optional comments in readXXX methods. With them it is much easier to understand where did checker stop its execution (if not you don't specify it, the comment will be like "integer doesn't belong to range \[23, 45\]" and it's not clear what integer caused such behavior). Also write informative comments in quitf calls; remember that your comments will be probably available to the contestants after the competition (or even during the competition like while hacking on Codeforces).