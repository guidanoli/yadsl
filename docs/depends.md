# Dependencies {#depends}

This is an *extremely picky* list of dependencies, which means that it is
very likely that older versions of such dependencies might still work.
If you found that an older version still works, don't hesitate in sending a
[pull request](https://github.com/guidanoli/yadsl/pulls).
This way, the list always is kept up-to-date.

## Supported Compilers

Since portability has always been an important aspect of **yadsl**, you should
be able to compile it in pretty much any compiler that mostly supports *C99*.
In that matter, following are the officially supported compilers.

* [GCC](https://gcc.gnu.org/) >= 5.4.0
* [Clang/LLMV](https://clang.llvm.org/) >= 7.0.0
* [Visual C++](https://visualstudio.microsoft.com/vs/) >= 16.7.2

## Build tools

Tools necessary to generate a build system, but not compile the source code.

* [CMake](https://cmake.org/) >= 3.0.0

## Documentation tools

Tools necessary to generate the library documentation, if you wish to.

* [Doxygen](https://www.doxygen.nl/index.html) >= 1.8.19

## Extension Languages

Languages that allow dynamic linking of extension modules binding **yadsl**.

* [CPython](https://www.python.org/) >= 3.6.11
* [Lua](https://www.lua.org/) >= 5.3.5