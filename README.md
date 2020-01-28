# Analysis of Algorithms (INF1721)
[![Build Status](https://travis-ci.com/guidanoli/aa.svg?branch=master)](https://travis-ci.com/guidanoli/aa)

This is a repository for toy projects that sparked from the lectures given by Eduardo Sany Laber, professor at PUC-Rio. I hope it comes to any help to you. Feel free to contribute to the repository too.

## Modules

Each subfolder generally contains a source file (`module.c`), a header file (`module.h`), a test module (`module.test.c`), and a documentation file (`README.md`), as well as a compilation file.

## Compilation

All the compilation process is described in form of `CMakeLists.txt`. Having installed [cmake](https://cmake.org/), simply run:

```bash
mkdir build
cd build
cmake ..
```

## Tests

All test modules work pretty much the same way, letting you interact with a single instance of the data structure through command line arguments. If you seek further information about a given test module, simply run the test module executable with no arguments whatsoever and you'll be prompted with some useful help messages.
