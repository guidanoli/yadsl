# tester

Framework designed for testing C modules.

## Motivation

Creating individual tester for each module can be a very boring process, which can also lead to wasting time on debugging the test module, but not on the module itself. Having a single tester framework makes the developing process so much easier, enjoyable and energetic.

## Paradigm

Instead of creating a test module entirely from scratch, which includes parsing, error detection, printing error and log information... You should only write code that is specific to the module you're testing, right? That is why you only need to implement three functions:

* `TesterInitCallback`: called to initialize data structures or global variables
* `TesterParseCallback`: called for every command parsed from the script file
* `TesterExitCallback`: called after an error is caught or at the very end, if no errors are throw

You may also want to display some helping information if no script file is passed to the tester, like what commands are available or how do the tester works. For that, you may also set the following variable:

* `TesterHelpStrings`: array of strings that will be printed when no arguments are passed

Following are some information about specific portions of the tester framework.

## Scripts

A script file has a very simple grammar, with the following tokens: **commands**, **arguments** and **comments**. These tokens are separated by ` spaces` and `tabs`, with exceptions (1), and by `newlines`.

* Commands are written as `/`  + `string`, preceding its arguments.
* Arguments are parsed as `int`,  `float`, `long` or `char *`.
* Comments are parsed as `#` + `any string`.

(1) `char *` arguments can be parsed with quotation marks, allowing `spaces` and `tabs` to be ignored

(2) comments ignore everything that comes after `#`, until the end of the line