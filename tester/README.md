# tester

Framework designed for testing C modules.

## Motivation

Creating individual tester modules for each module is boring and prone to errors that can lead to extra time debugging, which slows down quite a lot the development process.

## Scripts

The input is a script file which contains a list of lines. Lines can have many commands and an optional comment at the end. Commands can have arguments.

### Lines

Can have up to 1024 characters.

### Commands

Start with `/`, followed by the command name, which cannot have spacing characters.

#### Command arguments

Can be of three types:

* integers (signed or unsigned)
* floats (decimal, exponential or scientific notation)
* strings (quoted or unquoted)

**Disclaimer:** only quoted strings can have spacing characters

#### Command return values

You can use either the native return values, or so called "external" return values, which are custom return values defined by your tester implementation.

### Comments

Start with `#`, followed by any string of characters.

## Tester API

In order to implement a tester module, you must compile the tester framework together with a C file that implements all the undefined extern functions denoted in `tester.h`. Read it for further details, but be sure to check the `testertest` example.

If your project makes use of [cmake](https://cmake.org/), you can use the `add_tester_module` and `add_tester_scripts` functions available by running `add_subdirectory(tester)` on your parent `CMakeLists.txt`.