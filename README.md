# Analysis of Algorithms (INF1721)

This is a repository for toy projects that sparked from the lectures given by Eduardo Sany Laber, professor at PUC-Rio. I hope it comes to any help to you. Feel free to contribute to the repository too.

## Structure

Each subfolder generally contains a source file (`module.c`), a header file (`module.h`), a test module (`module.test.c`), and documentation (`README.md`).

## Compilation

All the compilation process is described in form of `CMakeLists.txt`. Having installed [cmake](https://cmake.org/), simply run:

```bash
mkdir build
cd build
cmake ..
```

## Tests

All test modules take work pretty much the same way. You'll be interaction with a single instance of the data structure through command line arguments. For further information on a given test module, simply run the test module executable with no arguments whatsoever.