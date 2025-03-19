_Section about testlib is temporary, at some day it will be merged into global documentation section when it appears._

If you are developing a programming contest problem and you are doing it with using C++ then testlib.h is a right choice to write all auxiliary programs. This library is a standard solution in a professional community of problemsetters around the world. Many contests are prepared using testlib.h: All-Russian and International olympiads in Informatics, ICPC regional contests, all Codeforces round and many other competitions.

You can find testlib.h [on GitHub](https://github.com/MikeMirzayanov/testlib).

testlib.h library is contained in a single header file. In order to use it you should just put testlib.h in the same directory with a program you are writing (checker, generator, validator, or interactor) and add the following line to the beginning of your program: `#include "testlib.h"`.

Here are the cases when testlib.h is really useful:

-   [In writing _generators_](/blog/entry/18291). These are the programs that create tests for your problem, since it is not always possible to type a whole content of the test by using the keyboard (at least because of their possible large size);
-   [In writing _validators_](/blog/entry/18426). These are programs that read the whole test and verifies that it is correct and that it satisfies the constraints of the problem. Validators should be maximally strict with respect to spaces, endlines, leading zeroes etc;
-   [In writing _interactors_](/blog/entry/18455). These are programs that are used in interactive problems, if your problem isn't interactive then just nevermind;
-   [In writing _checkers_](/blog/entry/18431). If your problem allows several possible answers for the tests then you should write a special program that checks participant's answer against jury's answer and the input data.

testlib.h is fully compatible with [Polygon](https://polygon.codeforces.com) problem preparation system.

First versions of testlib.h appeared in 2005 as a result of testlib.pas porting on C++. Since then testlib.h has evolved, its features and performance were improved. Last versions of testlib.h are compatible with different versions of Visual Studio compilers and GCC g++ (in editions for many platforms), also they are compatible with C++20.