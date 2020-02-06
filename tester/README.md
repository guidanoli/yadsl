# tester

Framework designed for testing C modules.

## Motivation

Creating individual testers for each module can be a very boring and error-prone process. It may lead to extra time wasted on debugging the test module, but not on the module itself. Having a single tester framework makes testing easier, enjoyable and energetic.

## Paradigm

Instead of creating a test module entirely from scratch, you should only write code that is specific to the module you're testing, right? That is why you only need to implement three functions:

* `TesterInitCallback()`: called to initialize data structures or global variables
* `TesterParseCallback(const char *)`: called for every command parsed from the script file
* `TesterExitCallback()`: called after an error is caught or at the very end, if no errors are thrown

You may also want to display some helping information if no script file is passed to the tester, like what commands are available or how does your test work. For that, you may also set the following variable:

* `TesterHelpStrings`: array of strings that will be printed when no arguments are passed

Following are some information about specific portions of the tester framework.

## Scripts

A script file has a very simple grammar, with the following tokens: **commands**, **arguments** and **comments**. These tokens are separated by ` spaces` and `tabs`, with exceptions (1)(2), and by `newlines`.

* Commands are written as `/`  + `string`, preceding its arguments.
* Arguments are parsed as `int`,  `float`, `long` or `char *`.
* Comments are parsed as `#` + `any string`.

(1) `char *` arguments can be parsed with quotation marks, allowing `spaces` and `tabs` to be ignored

(2) comments ignore everything that comes after `#`, until the end of the line

## Commands

Every time a command is parsed, the tester framework calls the `TesterParseCallback`, passing as argument the command string (without  `/`). It is the job of this function to parse this string (generally by `strcmp` with known commands), and requiring more parameters through `TesterParseArguments`, if necessary.

## Return value

All of the functions to be implemented must return `TesterReturnValue`. If any of these functions, when called, return a value other than `TESTER_RETURN_OK`, an error will be raised. In `tester.h` are listed the called "native" errors. These errors have their own string identifiers too. Its use will be soon explained.

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

You may have noticed that the `EXTERNAL` return value is the only one that does not have a string nor a description defined. That is due to the fact that it is a polymorphic return value. It can be whatever the user defines at runtime. Instead of returning the enumerator (`TESTER_RETURN_EXTERNAL`), you must call `TesterExternalReturnValue(const char *)`, passing its string as parameter, which then returns the enumerator.

## Error handling

There is only one way to catch errors raised by `TesterParseCallback`. It is by the `/catch` command, which must succeed the command that caused the error. Following the `/catch` command should be the error string. If the error string does not match the thrown error's string, the later will be thrown anyways.

## Example

In this folder are `tester.test.c` and `tester.script`, which exemplify the usage of the tester framework, as well as testing its main features, such as parsing and error handling. Be sure to consult them for reference.

