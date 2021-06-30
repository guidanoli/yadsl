# Dependencies {#depends}

This is an *extremely picky* list of dependencies, which means that it is
very likely that older versions of such dependencies might still work.
If you found that an older version still works, don't hesitate in sending a
[pull request](https://github.com/guidanoli/yadsl/pulls).
This way, the list always is kept up-to-date.

## Compiler

Since portability has always been an important aspect of **yadsl**, you should
be able to compile it in pretty much any C compiler that supports *C99*.

## Build tools

Tools necessary to generate a build system, but not compile the source code.

* [CMake](https://cmake.org/) >= 3.12

## Documentation tools

Tools necessary to generate the library documentation, if you wish to.

* [Doxygen](https://www.doxygen.nl/index.html) >= 1.8.19

## Extension Languages

Languages that allow dynamic linking of extension modules binding **yadsl**.

* [CPython](https://www.python.org/) >= 3.6
* [Lua](https://www.lua.org/) >= 5.4
