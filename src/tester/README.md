# tester

Framework designed for testing C modules.

## Motivation

Creating individual testers for each module can be a very dull and error-prone task. It may lead to extra time wasted on debugging the test module, but not the code itself. Having a single tester framework makes testing effortless, enjoyable and energetic.

## Paradigm

Instead of creating a test module entirely from scratch, you should only write code that is specific to what you're testing, right? That is why you only need to implement three functions, shown in order of activation.

* `TesterInitCallback()`: Called **once** to initialize data structures or global variables
* `TesterParseCallback(const char *)`: Called **for every command parsed** from the script file
* `TesterExitCallback()`: Called **once** at the very end of the testing cycle

You may also want to display some helping information if the tester receives no script file, like what commands are available or how does your test work. For that, you may also set the following variable:

* `TesterHelpStrings`: An array of strings terminated by `NULL`.

Following are some information about specific portions of the tester framework.

## Scripts

A script file has a very simple grammar, with the following tokens: **commands**, **arguments** and **comments**. These tokens are separated by ` spaces` and `tabs`, with exceptions (1)(2), and by `newlines`.

* **Commands** are written as `/`  + `string`, preceding its arguments.
* **Arguments** can be parsed as `int`,  `float`, `long`, `size_t` or `char *`.
* **Comments** are parsed as `#` + `any string`.

(1) `char *` arguments can be parsed with quotation marks, ignoring `spaces` and `tabs`.

(2) comments ignore everything that comes after `#`, until the end of the line

## Commands

The tester framework calls the `TesterParseCallback` for every potential command. It is the job of this function to parse this string, and requiring more parameters through `TesterParseArguments`, if necessary.

## Return value

All of the implementable functions return an enumerator of type `TesterReturnValue`. If any of these functions, when called, return a value other than `TESTER_RETURN_OK`, the corresponding cycle is interrupted. In `tester.h` are listed the called "native" errors. These errors have their string identifiers too, which are used for error handling, as we will see later on.

|      **enumerator**      | **string** |     **description**      |
| :----------------------: | :--------: | :----------------------: |
|    `TESTER_RETURN_OK`    |    `ok`    |    No errors occurred    |
|   `TESTER_RETURN_FILE`   |   `file`   |  Failed file operation   |
|  `TESTER_RETURN_MALLOC`  |  `malloc`  | Failed memory allocation |
| `TESTER_RETURN_MEMLEAK`  | `memleak`  |   Memory leak detected   |
| `TESTER_RETURN_OVERFLOW` | `overflow` |     Buffer overflow      |
| `TESTER_RETURN_COMMAND`  | `command`  |  Failed command parsing  |
| `TESTER_RETURN_ARGUMENT` | `argument` | Failed argument parsing  |
|  `TESTER_RETURN_RETURN`  |  `return`  |    Unexpected return     |
| `TESTER_RETURN_EXTERNAL` |   **?**    |          **?**           |

### External return values

You may have noticed that the `EXTERNAL` return value is the only one that does not have a string nor a description defined. That is because it is a polymorphic return value. It can be whatever the user determines at runtime. Instead of returning this value right away, you ought to call `TesterExternalReturnValue`, passing its string as the parameter, which then returns the enumerator.

## Error handling

There is only one way to catch errors raised by `TesterParseCallback`. It is by the `/catch` command. Following the `/catch` command should be the expected error string.

## Example

In this folder are `tester.test.c` and `tester.script`, which exemplify the usage of the tester framework, as well as testing its main features, such as parsing and error handling. Be sure to consult them for reference.

## Verbose

You can also compile the `testerlib` with the flag `_VERBOSE` which allows a more verbose output to help debug with more confidence.
