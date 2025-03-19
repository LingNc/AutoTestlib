If you have written some programming problems, and have prepared test cases, you will probably experience the terrible feeling that some test cases may be invalid (meaning it does not agree with the constraints in problem statement): upper bound can be violated, your graph not satisfied connectivity requirements or is not at tree... It is reasonable to feel that way. Even experienced problem setters make mistakes sometimes (for example, in the prestigious ACM ICPC World final 2007).

It is strictly recommended to write a special program (called _validator_) to formally check each test to satisfy all requirements from problem statements. Validators are strictly required for problems on Codeforces. [Polygon](https://polygon.codeforces.com/) has built-in support of validators.

It is really easy to write a validator using testlib.h.

## Example

Following is a validator that could be used for problem [100541A - Stock Market](https://codeforces.com/gym/100541/problem/A "2014 ACM-ICPC Vietnam National Second Round"):

**Original validator using an older version of testlib.h**

The wonderful thing about this validator is that it is very simple and it is very difficult to write something incorrect.

More examples can be found at [the Github repo](https://github.com/MikeMirzayanov/testlib/tree/master/validators)

## Available methods

The first line of your code should be `registerValidation(argc, argv)` which does some magic in the background, so that you can use the necessary methods. Most methods for validators are methods of the input stream `inf`, they start with prefix `read` and do the same thing: moves input stream pointer to next suitable place after reading something. It also detect violations (input does not match what you are trying to read), and then throw error. Usually the method's name is `inf.readT[s]`, where `T` is one of `Int`, `Long`, `Double`, `Token`, `Line`, `Space`, `Eoln`, `Eof`, etc, depending on the data you want to read, while the `s` suffix is needed when you want to read an array of similar data. The complete list of such functions is given at the end of this post. Apart from these functions, the following are useful:

| Method | What it does |
| --- | --- |
| `void registerValidation(argc, argv)` | This method must be called at the beginning of your code in order to use validator.  
After calling this method, you can access input stream by variable named `inf`. |
| `void setTestCase(int T)` | For problems with test cases, allows to automatically print the test case number _T_, where an error in the data is found. |
| `void unsetTestCase()` | Removes the printing of test case number on error. |
| `string validator.testset()` | Returns the name of the test set the validator is called for. When run from command line, use `--testset testcase_name` argument to set the test set name. |
| `string validator.group()` | Returns the name of the test group the validator is called for. When run from command line, use `--group group_name` argument to set the group name. |
| `void ensure(bool cond)` | Abort with an error when `cond` is not satisfied. |
| `void ensuref(bool cond, char *format, ...)` | Abort with an error when `cond` is not satisfied, additionally print a comment (see below). |

#### `variableName` argument

It is recommended to additionally pass a string argument called _variableName_ to `read*` functions to make the error message human-readable. So it is preferred to use `inf.readInt(1, 100, "n")` instead of `inf.readInt(1, 100)`. The first statement will fail with human-readable message like `FAIL Integer parameter [name=n] equals to 0, violates the range [1, 100]`.

#### Using `ensure`/`ensuref`

To check a requirement (like a graph doesn't contain loops, i. e. _x_<sub><i>i</i></sub> ≠ _y_<sub><i>i</i></sub>) use `ensuref(x_i != y_i, "Graph can't contain loops")`. It is allowed to use C-language format specifiers like `ensuref(s.length() % 2 == 0, "String 's' should have even length, but s.length()=%d", int(s.length()))`. Also you can use simple form like `ensure(x > y)`, it will print failed condition if it doesn't hold in form `FAIL Condition failed: "x > y"`.

#### Notes

-   Validator is strict. It cares about correct placing of spaces. For example, when you're trying to read an integer and the next character is a space (and then an integer), the validator will throw error.
-   Some methods have "regex" feature. It is not a full-featured regex as you may have used in many programming languages. It is a very simple version, which supports the following:
    -   **Set** of character, e.g: `[a-z]` is a lowercase latin letter, `[^a-z]` matches anything but a lowercase latin letter.
    -   **Range**, e.g. `[a-z]{1,5}` is a string of length 1 to 5 consists of only lowercase latin letter.
    -   **Or** operator, e.g. `mike|john` is either `mike` or `john`.
    -   **Optional** character, e.g. `-?[1-9][0-9]{0,3}` will match non-zero integers from -9999 to 9999 (note the optional minus sign).
    -   **Repetition**, e.g. `[0-9]*` will match sequences (empty or non-empty) of digits, and `[0-9]+` will match non-empty sequences of digits.
-   Also regarding regex, very simple greedy algorithm is used. For example, pattern `[0-9]?1` will not match `1`, because of greedy nature of matching.

Following is full list of methods of the input stream (`inf.<method>`):

| Method | What it does |
| --- | --- |
| char readChar() | Returns current character and moves pointer one character forward. |
| char readChar(char c) | Same as `readChar()` but ensures that the readCharacter is 'c'. |
| char readSpace() | Same as `readChar(' ')`. |
| void unreadChar(char c) | Puts back character c to input stream. |
| string readToken() | Reads a new token, i.e. a word that doesn't contain any whitespaces (like space, tab, EOLN and etc). |
| string readToken(string regex) | Same as `readToken()` but ensures that it matches given regex. |
| string readWord() | Same as `readToken()` |
| string readWord(string regex) | Same as `readToken(string regex)` |
| long long readLong() | Reads a long (long long in C/C++ and long in Java) |
| long long readLong(long long L, long long R) | Same as `readLong()` but ensures that the value is in range \[_L_, _R_\] (inclusively) |
| vector<long long> readLongs(int n, long long L, long long R) | Reads _n_ space separated longs (long long in C/C++ and long in Java) and ensures that the values are in range \[_L_, _R_\] (inclusively) |
| int readInt(),  
int readInteger() | Reads an integer (int type in both Java and C/C++) |
| int readInt(int L, int R),  
int readInteger(L, R) | Same as `readInt()` but ensures that the value is in range \[_L_, _R_\] (inclusively) |
| vector<int> readInts(int n, int L, int R),  
vector<int> readIntegers(int n, int L, int R) | Reads _n_ space separated integers and ensures that the values are in range \[_L_, _R_\] (inclusively) |
| double readReal(),  
double readDouble() | Reads a double. |
| double readReal(double L, double R),  
double readDouble(double L, double R) | Same as `readReal()`, `readDouble()` but ensures that the value is in range \[_L_, _R_\]. |
| double readStrictReal(double L, double R, int minPrecision, int maxPrecision),  
double readStrictDouble(double L, double R, int minPrecision, int maxPrecision) | Same as `readReal(L, R)`, `readDouble(L, R)`, but additionally ensures that the number of digits after decimal point is between \[_minPrecision_, _maxPrecision_\]. Doesn't allow exponential or any other non-standard form. |
| string readString(),  
string readLine() | Reads a line from current position to EOLN. Moves input stream pointer to first character of new line (if exists). |
| string readString(string regex),  
string readLine(string regex) | Same as `readString()` and `readLine()`, but ensures that the string matches given regex. |
| void readEoln() | Reads EOLN or fails. Note that this method magically works on both Windows and Linux. On Windows it reads #13#10 and on Linux it reads #10. |
| void readEof() | Reads EOF or fails. |

Reference: [Github page of testlib.h](https://github.com/MikeMirzayanov/testlib/blob/master/testlib.h)