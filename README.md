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

All test modules implement the specific functions of the `tester` framework. To read more about it, go to `tester/README.md`. In order to run the tests, assuming you have [cmake](https://cmake.org/) installed, simply run:

```bash
cd build
ctest -C <CONFIG>
```

Where `CONFIG` stands for the project configuration (like `Debug` or `Release`).